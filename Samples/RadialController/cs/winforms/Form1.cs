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
            radialController.Menu.Items.Add(RadialControllerMenuItem.CreateFromKnownIcon("My Ruler", RadialControllerMenuKnownIcon.Ruler));

            AddItemFromImageFile();
        }

        private void AddItemFromImageFile()
        {
            string iconFileName = "Item0.png";
            string filePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, iconFileName);
            var getItemImageOperation = StorageFile.GetFileFromPathAsync(filePath);

            getItemImageOperation.Completed += new AsyncOperationCompletedHandler<StorageFile>(AddMenuItemFromImage);
        }

        private void AddMenuItemFromImage(IAsyncOperation<StorageFile> asyncInfo, AsyncStatus asyncStatus)
        {
            if (asyncStatus == AsyncStatus.Completed)
            {
                StorageFile imageFile = asyncInfo.GetResults();
                radialController.Menu.Items.Add(RadialControllerMenuItem.CreateFromIcon("test", RandomAccessStreamReference.CreateFromFile(imageFile)));
            }
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
    }
}
