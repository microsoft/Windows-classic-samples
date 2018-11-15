#include "stdafx.h"
#include "SpatialInputHandler.h"
#include <functional>

// include at top of file
#include <SpatialInteractionManagerInterop.h>
#include <windows.ui.input.spatial.h>
#include <winrt\windows.ui.input.spatial.h>

using namespace BasicHologram;

using namespace std::placeholders;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Input::Spatial;

// Creates and initializes a GestureRecognizer that listens to a Person.
SpatialInputHandler::SpatialInputHandler(HWND hWnd)
{
    // The interaction manager provides an event that informs the app when
    // spatial interactions are detected.
    {
        winrt::com_ptr<ISpatialInteractionManagerInterop> spatialInteractionManagerInterop = winrt::get_activation_factory<SpatialInteractionManager, ISpatialInteractionManagerInterop>();

        if (!spatialInteractionManagerInterop)
        {
            winrt::check_hresult(E_FAIL);
        }

        winrt::com_ptr<ABI::Windows::UI::Input::Spatial::ISpatialInteractionManager> spSpatialInteractionManager;
        winrt::check_hresult(spatialInteractionManagerInterop->GetForWindow(hWnd, __uuidof(ABI::Windows::UI::Input::Spatial::ISpatialInteractionManager), winrt::put_abi(spSpatialInteractionManager)));

        if (!spSpatialInteractionManager)
        {
            winrt::check_hresult(E_FAIL);
        }

        m_interactionManager = spSpatialInteractionManager.as<SpatialInteractionManager>();
    }

    // Bind a handler to the SourcePressed event.
    m_sourcePressedEventToken = m_interactionManager.SourcePressed(bind(&SpatialInputHandler::OnSourcePressed, this, _1, _2));

    //
    // TODO: Expand this class to use other gesture-based input events as applicable to
    //       your app.
    //
}

SpatialInputHandler::~SpatialInputHandler()
{
    // Unregister our handler for the OnSourcePressed event.
    m_interactionManager.SourcePressed(m_sourcePressedEventToken);
}

// Checks if the user performed an input gesture since the last call to this method.
// Allows the main update loop to check for asynchronous changes to the user
// input state.
SpatialInteractionSourceState SpatialInputHandler::CheckForInput()
{
    SpatialInteractionSourceState sourceState = m_sourceState;
    m_sourceState = nullptr;
    return sourceState;
}

void SpatialInputHandler::OnSourcePressed(SpatialInteractionManager const& /*sender*/, SpatialInteractionSourceEventArgs const& args)
{
    m_sourceState = args.State();

    //
    // TODO: In your app or game engine, rewrite this method to queue
    //       input events in your input class or event handler.
    //
}
