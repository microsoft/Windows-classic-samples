using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using Windows.UI.Input;
using Windows.Storage.Streams;
using Windows.Storage;
using Windows.Foundation;
using Windows.Foundation.Metadata;

namespace RadialControllerWinForms
{
    public partial class Form1 : Form
    {
        RadialController radialController;
        private float _angle;

        public Form1()
        {
            InitializeComponent();

            _angle = 0;

            this.DoubleBuffered = true;
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);

            CreateController();
            SubscribeToControllerCallbacks();

            AddCustomItems();
            SetDefaultItems();
        }

        private void CreateController()
        {
            IRadialControllerInterop interop = (IRadialControllerInterop)System.Runtime.InteropServices.WindowsRuntime.WindowsRuntimeMarshal.GetActivationFactory(typeof(RadialController));
            Guid guid = typeof(RadialController).GetInterface("IRadialController").GUID;

            radialController = interop.CreateForWindow(this.Handle, ref guid);
        }

        private void SubscribeToControllerCallbacks()
        {
            radialController.ButtonClicked += RadialController_ButtonClicked;
            radialController.ControlAcquired += RadialController_ControlAcquired;
            radialController.RotationChanged += RadialController_RotationChanged;
        }

        private void AddCustomItems()
        {
            radialController.Menu.Items.Add(RadialControllerMenuItem.CreateFromKnownIcon("Ruler", RadialControllerMenuKnownIcon.Ruler));

            AddItemFromImage();
            AddItemsFromFont();
        }

        private void AddItemsFromFont()
        {
            // Using custom font
            string fontFilePath = GetSharedFilePath("Symbols.ttf");
            radialController.Menu.Items.Add(RadialControllerMenuItem.CreateFromFontGlyph("Custom Font Icon", "\xe102", "Symbols", new Uri(fontFilePath)));

            // Using system font
            radialController.Menu.Items.Add(RadialControllerMenuItem.CreateFromFontGlyph("System Font Icon", "\xD83D\xDC31\x200D\xD83D\xDC64", "Segoe UI Emoji"));
        }

        private void AddItemFromImage()
        {
            string iconFilePath = GetFilePath("Palette.png");
            var getItemImageOperation = StorageFile.GetFileFromPathAsync(iconFilePath);

            getItemImageOperation.Completed += new AsyncOperationCompletedHandler<StorageFile>(OnImageFileFound);
        }

        private void OnImageFileFound(IAsyncOperation<StorageFile> asyncInfo, AsyncStatus asyncStatus)
        {
            if (asyncStatus == AsyncStatus.Completed)
            {
                StorageFile imageFile = asyncInfo.GetResults();
                radialController.Menu.Items.Add(RadialControllerMenuItem.CreateFromIcon("Custom Image", RandomAccessStreamReference.CreateFromFile(imageFile)));
            }
        }

        private string GetSharedFilePath(string fileName)
        {
            string projectDirectory = Directory.GetParent(Directory.GetCurrentDirectory()).Parent.Parent.Parent.FullName;
            string sharedFolderName = "shared";
            return Path.Combine(projectDirectory, sharedFolderName, fileName);
        }

        private string GetFilePath(string fileName)
        {
            string projectDirectory = Directory.GetParent(Directory.GetCurrentDirectory()).Parent.FullName;
            return Path.Combine(projectDirectory, fileName);
        }

        private void SetDefaultItems()
        {
            RadialControllerConfiguration radialControllerConfig;
            IRadialControllerConfigurationInterop radialControllerConfigInterop = (IRadialControllerConfigurationInterop)System.Runtime.InteropServices.WindowsRuntime.WindowsRuntimeMarshal.GetActivationFactory(typeof(RadialControllerConfiguration));
            Guid guid = typeof(RadialControllerConfiguration).GetInterface("IRadialControllerConfiguration").GUID;

            radialControllerConfig = radialControllerConfigInterop.GetForWindow(this.Handle, ref guid);
            radialControllerConfig.SetDefaultMenuItems(new[] { RadialControllerSystemMenuItemKind.Volume, RadialControllerSystemMenuItemKind.Scroll });
            radialControllerConfig.TrySelectDefaultMenuItem(RadialControllerSystemMenuItemKind.Scroll);
        }

        private void RadialController_RotationChanged(RadialController sender, RadialControllerRotationChangedEventArgs args)
        {
            Rotation(args.RotationDeltaInDegrees);
        }

        private void RadialController_ControlAcquired(RadialController sender, RadialControllerControlAcquiredEventArgs args)
        {
        }

        private void RadialController_ButtonClicked(RadialController sender, RadialControllerButtonClickedEventArgs args)
        {
            Rotation(180);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            float cx = ClientRectangle.Width / 2;
            float cy = ClientRectangle.Height / 2;

            float r = Math.Min(ClientRectangle.Width, ClientRectangle.Height) - 10;

            float nx = (float)Math.Cos(_angle * (Math.PI / 180.0)) * r + cx;
            float ny = (float)Math.Sin(_angle * (Math.PI / 180.0)) * r + cy;
            e.Graphics.DrawLine(Pens.Black, cx, cy, nx, ny);
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            Invalidate();
        }

        private void Rotation(double delta)
        {
            _angle += (float)delta;

            if (_angle > 360f)
            {
                _angle -= 360f;
            }
            else if (_angle < 0f)
            {
                _angle += 360f;
            }

            Invalidate();
        }

        private void MenuSuppressionCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            RadialControllerConfiguration radialControllerConfig;
            IRadialControllerConfigurationInterop radialControllerConfigInterop = (IRadialControllerConfigurationInterop)System.Runtime.InteropServices.WindowsRuntime.WindowsRuntimeMarshal.GetActivationFactory(typeof(RadialControllerConfiguration));
            Guid guid = typeof(RadialControllerConfiguration).GetInterface("IRadialControllerConfiguration").GUID;

            radialControllerConfig = radialControllerConfigInterop.GetForWindow(this.Handle, ref guid);
            radialControllerConfig.ActiveControllerWhenMenuIsSuppressed = radialController;
            radialControllerConfig.IsMenuSuppressed = MenuSuppressionCheckBox.Checked;
        }
    }
}
