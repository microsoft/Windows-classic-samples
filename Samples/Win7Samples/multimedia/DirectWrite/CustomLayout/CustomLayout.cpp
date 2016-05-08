// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// Contents:    Main user interface window.
//
//----------------------------------------------------------------------------
#include "common.h"
#include "resource.h"
#include "FlowLayout.h"
#include "CustomLayout.h"

////////////////////////////////////////
// Main entry.

const wchar_t* MainWindow::g_windowClassName = L"DirectWriteCustomLayoutDemo";

// Shows an error message if the function returned a failing HRESULT,
// then returning that same error code.
HRESULT ShowMessageIfFailed(HRESULT functionResult, const wchar_t* message);


int APIENTRY wWinMain(
    HINSTANCE   hInstance, 
    HINSTANCE   hPrevInstance,
    LPWSTR      commandLine,
    int         nCmdShow
    )
{
    // The Microsoft Security Development Lifecycle recommends that all
    // applications include the following call to ensure that heap corruptions
    // do not go unnoticed and therefore do not introduce opportunities
    // for security exploits.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    MainWindow app;

    HRESULT hr = S_OK;
    hr = app.Initialize();

    if (SUCCEEDED(hr))
        hr = static_cast<HRESULT>(app.RunMessageLoop());

    return 0;
}


HRESULT MainWindow::Initialize()
{
    HRESULT hr = S_OK;

    //////////////////////////////
    // Create the DWrite factory.

    hr = ShowMessageIfFailed(
            DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&dwriteFactory_)
                ),
            L"Could not create DirectWrite factory! DWriteCreateFactory()" FAILURE_LOCATION
        );

    //////////////////////////////
    // Create the main window

    if (SUCCEEDED(hr))
    {
        MainWindow::RegisterWindowClass();

        CreateWindow(
                g_windowClassName,
                TEXT(APPLICATION_TITLE),
                WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,
                CW_USEDEFAULT, CW_USEDEFAULT,
                800,
                600,
                NULL,
                NULL,
                HINST_THISCOMPONENT,
                this
                );

        if (hwnd_ == NULL)
        {
            hr = ShowMessageIfFailed(
                    HRESULT_FROM_WIN32(GetLastError()),
                    L"Could not create main demo window! CreateWindow()"  FAILURE_LOCATION
                );
        }
        else
        {
            ShowWindow(hwnd_, SW_SHOWNORMAL);
            UpdateWindow(hwnd_);
        }
    }

    //////////////////////////////
    // Initialize the render target.

    if (SUCCEEDED(hr))
    {
        IDWriteGdiInterop* gdiInterop = NULL;

        hr = dwriteFactory_->GetGdiInterop(&gdiInterop);

        if (SUCCEEDED(hr))
        {
            RECT clientRect;
            GetClientRect(hwnd_, &clientRect);

            HDC hdc = GetDC(hwnd_);

            hr = ShowMessageIfFailed(
                    gdiInterop->CreateBitmapRenderTarget(hdc, clientRect.right, clientRect.bottom, &renderTarget_),
                    L"Could not create render target! CreateBitmapRenderTarget()" FAILURE_LOCATION
                    );

            ReleaseDC(hwnd_, hdc);
        }
        SafeRelease(&gdiInterop);
    }

    //////////////////////////////
    // Create our custom layout, source, and sink.

    if (SUCCEEDED(hr))
    {
        SafeSet(&flowLayoutSource_, new(std::nothrow) FlowLayoutSource);
        SafeSet(&flowLayoutSink_,   new(std::nothrow) FlowLayoutSink);
        SafeSet(&flowLayout_,       new(std::nothrow) FlowLayout(dwriteFactory_));

        if (flowLayoutSource_ == NULL
        ||  flowLayoutSink_   == NULL
        ||  flowLayout_       == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        SetLayoutText(CommandIdTextLatin);
        SetLayoutShape(CommandIdShapeCircle);

        OnMove();
        OnSize(); // update size and reflow

        InvalidateRect(hwnd_, NULL, FALSE);
    }

    return hr;
}


ATOM MainWindow::RegisterWindowClass()
{
    // Registers the main window class.

    WNDCLASSEX wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
    wcex.lpfnWndProc   = &WindowProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = sizeof(LONG_PTR);
    wcex.hInstance     = HINST_THISCOMPONENT;
    wcex.hIcon         = NULL;
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName  = MAKEINTRESOURCE(1);
    wcex.lpszClassName = g_windowClassName;
    wcex.hIconSm       = NULL;

    return RegisterClassEx(&wcex);
}


WPARAM MainWindow::RunMessageLoop()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}


LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Relays messages for the main window to the internal class.

    MainWindow* window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_NCCREATE:
        {
            // Associate the data structure with this window handle.
            CREATESTRUCT* pcs = reinterpret_cast<CREATESTRUCT*>(lParam);
            window = reinterpret_cast<MainWindow*>(pcs->lpCreateParams);
            window->hwnd_ = hwnd;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, PtrToUlong(window));
        }
        return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            window->OnPaint(ps);
            EndPaint(hwnd, &ps);
        }
        break;

    case WM_PRINTCLIENT:
        {
            PAINTSTRUCT ps = {};
            ps.hdc = (HDC)wParam;
            GetClientRect(hwnd, &ps.rcPaint);
            window->OnPaint(ps);
        }
        break;

    case WM_ERASEBKGND: // don't want flicker
        return true;

    case WM_COMMAND:
        window->OnCommand(static_cast<UINT>(wParam));
        break;

    case WM_SIZE:
        window->OnSize();
        break;

    case WM_MOVE:
        window->OnMove();
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}


void MainWindow::OnCommand(UINT commandId)
{
    // Handles menu commands.

    switch (commandId)
    {
    case CommandIdShapeFunnel:
    case CommandIdShapeCircle:
        SetLayoutShape(commandId);
        ReflowLayout();
        break;

    case CommandIdNumbersNominal:
    case CommandIdNumbersArabic:
        SetLayoutNumbers(commandId);
        ReflowLayout();
        break;

    case CommandIdTextLatin:
    case CommandIdTextArabic: 
    case CommandIdTextJapanese:
        SetLayoutText(commandId);
        ReflowLayout();
        break;

    case IDCLOSE:
        PostMessage(hwnd_, WM_CLOSE, 0,0);
        break;
    }
}


void MainWindow::OnPaint(const PAINTSTRUCT& ps)
{
    // Redraws the glyph runs.

    if (renderTarget_ == NULL)
        return;

    HDC memoryHdc = renderTarget_->GetMemoryDC();

    // Clear background.
    SetDCBrushColor(memoryHdc, GetSysColor(COLOR_WINDOW));
    SelectObject(memoryHdc, GetStockObject(NULL_PEN));
    SelectObject(memoryHdc, GetStockObject(DC_BRUSH));
    Rectangle(memoryHdc, ps.rcPaint.left,ps.rcPaint.top, ps.rcPaint.right, ps.rcPaint.bottom);

    // Draw all of the produced glyph runs.
    flowLayoutSink_->DrawGlyphRuns(renderTarget_, renderingParams_, GetSysColor(COLOR_WINDOWTEXT));

    // Transfer drawn image to display.
    BitBlt(
        ps.hdc,
        ps.rcPaint.left,
        ps.rcPaint.top,
        ps.rcPaint.right  - ps.rcPaint.left,
        ps.rcPaint.bottom - ps.rcPaint.top,
        memoryHdc,
        ps.rcPaint.left,
        ps.rcPaint.top,
        SRCCOPY | NOMIRRORBITMAP
        );
}


void MainWindow::OnSize()
{
    // Resizes the render target and flow source.

    RECT rect;
    GetClientRect(hwnd_, &rect);

    if (renderTarget_ == NULL)
        return;

    renderTarget_->Resize(rect.right, rect.bottom);

    if (flowLayoutSource_ == NULL)
        return;

    float pixelsPerDip = renderTarget_->GetPixelsPerDip();
    flowLayoutSource_->SetSize(float(rect.right) / pixelsPerDip, float(rect.bottom) / pixelsPerDip);
    ReflowLayout();
}


void MainWindow::OnMove()
{
    // Updates rendering parameters according to current monitor.

    if (dwriteFactory_ == NULL)
        return; // Not initialized yet.

    HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
    if (monitor == hmonitor_)
        return; // Still on previous monitor.

    SafeRelease(&renderingParams_);
    dwriteFactory_->CreateMonitorRenderingParams(
                    MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST),
                    &renderingParams_
                    );

    if (renderingParams_ == NULL)
        return;

    hmonitor_ = monitor;
    InvalidateRect(hwnd_, NULL, FALSE);
}


STDMETHODIMP MainWindow::ReflowLayout()
{
    // Reflows the layout after a resize or text change.

    HRESULT hr = S_OK;

    if (FAILED(hr = flowLayoutSource_->Reset()))
        return hr;

    if (FAILED(hr = flowLayoutSink_->Reset()))
        return hr;
    
    if (FAILED(hr = flowLayout_->FlowText(flowLayoutSource_, flowLayoutSink_)))
        return hr;

    InvalidateRect(hwnd_, NULL, false);
    return hr;
}


STDMETHODIMP MainWindow::SetLayoutText(UINT commandId)
{
    // Selects a different text sample.

    HRESULT hr = S_OK;

    DWRITE_READING_DIRECTION readingDirection = DWRITE_READING_DIRECTION_LEFT_TO_RIGHT;
    const wchar_t* text         = L"";
    const wchar_t* fontName     = L"";
    const wchar_t* localeName   = L"";

    switch (commandId)
    {
    case CommandIdTextLatin:
        fontName = L"Segoe UI";
        localeName = L"en-us";
        text =
            L"DirectWrite provides factored layers of functionality, with each layer interacting seamlessly with the next. "
            L"The API design gives an application the freedom and flexibility to adopt individual layers depending on their needs and schedule.\n"
            L"\n"
            L"The text layout API provides the highest level functionality available from DirectWrite. "
            L"It provides services for the application to measure, display, and interact with richly formatted text strings. "
            L"This text API can be used in applications that currently use Win32’s DrawText to build a modern UI with richly formatted text.\n"
            L"\n"
            L"* Text-intensive applications that implement their own layout engine may use the next layer down: the script processor. "
            L"The script processor segments text into runs of similar properties and handles the mapping from Unicode codepoints "
            L"to the appropriate glyph in the font. "
            L"DirectWrite's own layout is built upon this same font and script processing system. "
            L"This sample demonstrates how a custom layout can utilize the information from script itemization, bidi analysis, line breaking analysis, and shaping, "
            L"to accomplish text measurement/fitting, line breaking, basic justification, and drawing.\n"
            L"\n"
            L"The glyph-rendering layer is the lowest layer and provides glyph-rendering functionality for applications "
            L"that implement their own complete text layout engine. The glyph rendering layer is also useful for applications that implement a custom "
            L"renderer to modify the glyph-drawing behavior through the callback function in the DirectWrite text-formatting API.\n"
            L"\n"
            L"The DirectWrite font system is available to all the functional layers, and enables an application to access font and glyph information. "
            L"It is designed to handle common font technologies and data formats. The DirectWrite font model follows the common typographic practice of "
            L"supporting any number of weights, styles, and stretches in the same font family. This model, the same model followed by WPF and CSS, "
            L"specifies that fonts differing only in weight (bold, light, etc.), style (upright, italic, or oblique) or stretch (narrow, condensed, wide, etc.) "
            L"are considered to be members of a single font family.\n"
            L"\n"
            L"Text in DirectWrite is rendered using Microsoft® ClearType®, which enhances the clarity and readability of text. "
            L"ClearType takes advantage of the fact that modern LCD displays have RGB stripes for each pixel that can be controlled individually. "
            L"DirectWrite uses the latest enhancements to ClearType, first included with Windows Vista® with Windows Presentation Foundation, "
            L"that enables it to evaluate not just the individual letters but also the spacing between letters. "
            L"Before these ClearType enhancements, text with a “reading” size of 10 or 12 points was difficult to display: "
            L"we could place either 1 pixel in between letters, which was often too little, or 2 pixels, which was often too much. "
            L"Using the extra resolution in the subpixels provides us with fractional spacing, which improves the evenness and symmetry of the entire page.\n"
            L"\n"
            L"The subpixel ClearType positioning offers the most accurate spacing of characters on screen, "
            L"especially at small sizes where the difference between a sub-pixel and a whole pixel represents a significant proportion of glyph width. "
            L"It allows text to be measured in ideal resolution space and rendered at its natural position at the LCD color stripe, subpixel granularity. "
            L"Text measured and rendered using this technology is, by definition, "
            L"resolution-independent—meaning the exact same layout of text is achieved across the range of various display resolutions.\n"
            L"\n"
            L"Unlike either flavor of GDI's ClearType rendering, sub-pixel ClearType offers the most accurate width of characters. "
            L"The Text String API adopts sub-pixel text rendering by default, which means it measures text at its ideal resolution independent "
            L"to the current display resolution, and produces the glyph positioning result based on the truly scaled glyph advance widths and positioning offsets.";
        break;

    case CommandIdTextArabic:
        fontName = L"Arabic Typesetting";
        localeName = L"ar-eg";
        text =
            L"الديباجة\n"
            L"لمّا كان الاعتراف بالكرامة المتأصلة في جميع أعضاء الأسرة البشرية وبحقوقهم المتساوية الثابتة هو أساس الحرية والعدل والسلام في العالم.\n"
            L"\n"
            L"ولما كان تناسي حقوق الإنسان وازدراؤها قد أفضيا إلى أعمال همجية آذت الضمير الإنساني. وكان غاية ما يرنو إليه عامة البشر انبثاق عالم يتمتع فيه الفرد بحرية القول والعقيدة ويتحرر من الفزع والفاقة.\n"
            L"\n"
            L"ولما كان من الضروري أن يتولى القانون حماية حقوق الإنسان لكيلا يضطر المرء آخر الأمر إلى التمرد على الاستبداد والظلم.\n"
            L"\n"
            L"ولما كانت شعوب الأمم المتحدة قد أكدت في الميثاق من جديد إيمانها بحقوق الإنسان الأساسية وبكرامة الفرد وقدره وبما للرجال والنساء من حقوق متساوية وحزمت أمرها على أن تدفع بالرقي الاجتماعي قدمًا وأن ترفع مستوى الحياة في جو من الحرية أفسح.\n"
            L"\n"
            L"ولما كانت الدول الأعضاء قد تعهدت بالتعاون مع الأمم المتحدة على ضمان إطراد مراعاة حقوق الإنسان والحريات الأساسية واحترامها.\n"
            L"\n"
            L"ولما كان للإدراك العام لهذه الحقوق والحريات الأهمية الكبرى للوفاء التام بهذا التعهد.\n"
            L"\n"
            L"فإن الجمعية العامة\n"
            L"\n"
            L"تنادي بهذا الإعلان العالمي لحقوق الإنسان\n"
            L"\n"
            L"على أنه المستوى المشترك الذي ينبغي أن تستهدفه كافة الشعوب والأمم حتى يسعى كل فرد وهيئة في المجتمع، واضعين على الدوام هذا الإعلان نصب أعينهم، إلى توطيد احترام هذه الحقوق والحريات عن طريق التعليم والتربية واتخاذ إجراءات مطردة، قومية وعالمية، لضمان الإعتراف بها ومراعاتها بصورة عالمية فعالة بين الدول الأعضاء ذاتها وشعوب البقاع الخاضعة لسلطانها.\n"
            L"\n"
            L"المادة 1\n"
            L"\n"
            L"يولد جميع الناس أحرارًا متساوين في الكرامة والحقوق. وقد وهبوا عقلاً وضميرًا وعليهم أن يعامل بعضهم بعضًا بروح الإخاء.\n"
            L"\n"
            L"المادة 2\n"
            L"\n"
            L"لكل إنسان حق التمتع بكافة الحقوق والحريات الواردة في هذا الإعلان، دون أي تمييز، كالتمييز بسبب العنصر أو اللون أو الجنس أو اللغة أو الدين أو الرأي السياسي أو أي رأي آخر، أو الأصل الوطني أو الإجتماعي أو الثروة أو الميلاد أو أي وضع آخر، دون أية تفرقة بين الرجال والنساء.\n"
            L"\n"
            L"وفضلاً عما تقدم فلن يكون هناك أي تمييز أساسه الوضع السياسي أو القانوني أو الدولي لبلد أو البقعة التي ينتمي إليها الفرد سواء كان هذا البلد أو تلك البقعة مستقلاً أو تحت الوصاية أو غير متمتع بالحكم الذاتي أو كانت سيادته خاضعة لأي قيد من القيود.\n";

        readingDirection = DWRITE_READING_DIRECTION_RIGHT_TO_LEFT;
        break;

    case CommandIdTextJapanese:
        fontName = L"Meiryo";
        localeName = L"jp-jp";
        text =
            L"『世界人権宣言』\n"
            L"（1948.12.10 第３回国連総会採択）〈前文〉\n"
            L"\n"
            L"人類社会のすべての構成員の固有の尊厳と平等で譲ることのできない権利とを承認することは、世界における自由、正義及び平和の基礎であるので、\n"
            L"\n"
            L"人権の無視及び軽侮が、人類の良心を踏みにじった野蛮行為をもたらし、言論及び信仰の自由が受けられ、恐怖及び欠乏のない世界の到来が、一般の人々の最高の願望として宣言されたので、\n"
            L"\n"
            L"人間が専制と圧迫とに対する最後の手段として反逆に訴えることがないようにするためには、法の支配によって人権を保護することが肝要であるので、\n"
            L"\n"
            L"諸国間の友好関係の発展を促進することが肝要であるので、\n"
            L"\n"
            L"国際連合の諸国民は、国連憲章において、基本的人権、人間の尊厳及び価値並びに男女の同権についての信念を再確認し、かつ、一層大きな自由のうちで社会的進歩と生活水準の向上とを促進することを決意したので、\n"
            L"\n"
            L"加盟国は、国際連合と協力して、人権及び基本的自由の普遍的な尊重及び遵守の促進を達成することを誓約したので、\n"
            L"\n"
            L"これらの権利及び自由に対する共通の理解は、この誓約を完全にするためにもっとも重要であるので、\n"
            L"\n"
            L"よって、ここに、国連総会は、\n"
            L"\n"
            L"\n"
            L"社会の各個人及び各機関が、この世界人権宣言を常に念頭に置きながら、加盟国自身の人民の間にも、また、加盟国の管轄下にある地域の人民の間にも、これらの権利と自由との尊重を指導及び教育によって促進すること並びにそれらの普遍的措置によって確保することに努力するように、すべての人民とすべての国とが達成すべき共通の基準として、この人権宣言を公布する。\n"
            L"\n"
            L"第１条\n"
            L"すべての人間は、生まれながらにして自由であり、かつ、尊厳と権利と について平等である。人間は、理性と良心とを授けられており、互いに同 胞の精神をもって行動しなければならない。\n"
            L"\n"
            L"第２条"
            L"すべて人は、人種、皮膚の色、性、言語、宗教、政治上その他の意見、\n"
            L"\n"
            L"国民的もしくは社会的出身、財産、門地その他の地位又はこれに類するい\n"
            L"\n"
            L"かなる自由による差別をも受けることなく、この宣言に掲げるすべての権\n"
            L"\n"
            L"利と自由とを享有することができる。\n"
            L"\n"
            L"さらに、個人の属する国又は地域が独立国であると、信託統治地域で\n"
            L"\n"
            L"あると、非自治地域であると、又は他のなんらかの主権制限の下にあると\n"
            L"\n"
            L"を問わず、その国又は地域の政治上、管轄上又は国際上の地位に基ずくい\n"
            L"\n"
            L"かなる差別もしてはならない。\n"
            L"\n"
            L"第３条\n"
            L"すべての人は、生命、自由及び身体の安全に対する権利を有する。\n"
            L"\n"
            L"第４条\n"
            L"何人も、奴隷にされ、又は苦役に服することはない。奴隷制度及び奴隷\n"
            L"\n"
            L"売買は、いかなる形においても禁止する。\n"
            L"\n"
            L"第５条\n"
            L"何人も、拷問又は残虐な、非人道的なもしくは屈辱的な取扱もしくは刑\n"
            L"\n"
            L"罰を受けることはない。\n";
        break;

    default:
        return E_FAIL;
    }
    textMode_ = commandId;

    IDWriteTextFormat* textFormat = NULL;
    hr = ShowMessageIfFailed(
            dwriteFactory_->CreateTextFormat(
                fontName,
                NULL,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                14, // fontSize
                localeName,
                &textFormat
                ),
            L"Could not create text format for custom layout! CreateTextFormat()"  FAILURE_LOCATION
            );

    if (SUCCEEDED(hr))
    {
        textFormat->SetReadingDirection(readingDirection);

        hr = ShowMessageIfFailed(
                flowLayout_->SetTextFormat(textFormat),
                L"Could not set text format on custom layout! SetTextFormat()"  FAILURE_LOCATION
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = ShowMessageIfFailed(
                flowLayout_->AnalyzeText(text, static_cast<UINT32>(wcsnlen(text, UINT32_MAX))),
                L"Text analysis failed! FlowLayout::AnalyzeText()"  FAILURE_LOCATION
                );
    }

    SafeRelease(&textFormat);

    return hr;
}


STDMETHODIMP MainWindow::SetLayoutShape(UINT commandId)
{
    return flowLayoutSource_->SetShape(FlowLayoutSource::FlowShape(commandId - CommandIdShapeFirstId));
}


STDMETHODIMP MainWindow::SetLayoutNumbers(UINT commandId)
{
    // Creates a number substitution to select which digits are displayed.

    HRESULT hr = S_OK;

    const wchar_t* localeName;
    switch (commandId)
    {
    case CommandIdNumbersNominal:
        localeName = L"en-us";
        break;

    case CommandIdNumbersArabic:
        localeName = L"ar-eg";
        break;

    default:
        return E_FAIL;
    }

    // Create and set the new digits.
    IDWriteNumberSubstitution* numberSubstitution = NULL;
    if (SUCCEEDED(hr))
    {
        hr = dwriteFactory_->CreateNumberSubstitution(DWRITE_NUMBER_SUBSTITUTION_METHOD_CONTEXTUAL, localeName, TRUE, &numberSubstitution);
        flowLayout_->SetNumberSubstitution(numberSubstitution);
    }

    hr = SetLayoutText(textMode_);

    SafeRelease(&numberSubstitution);

    return hr;
}


HRESULT ShowMessageIfFailed(HRESULT functionResult, const wchar_t* message)
{
    // Displays an error message for API failures,
    // returning the very same error code that came in.

    if (FAILED(functionResult))
    {
        const wchar_t* format = L"%s\r\nError code = %X";

        wchar_t buffer[1000];
        buffer[0] = '\0';

        StringCchPrintf(buffer, ARRAYSIZE(buffer), format, message, functionResult);
        MessageBox(NULL, buffer, TEXT(APPLICATION_TITLE), MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL);
    }
    return functionResult;
}
