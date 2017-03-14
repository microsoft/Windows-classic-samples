using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Windows.ApplicationModel.DataTransfer;
using Windows.Storage;

namespace WpfShareSource
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public enum ShareMode
        {
            Blank,
            Text,
            Weblink,
            StorageItem
        }

        ShareMode currentShareMode = ShareMode.Blank;

        public MainWindow()
        {
            InitializeComponent();
        }

        void Window_Loaded(object sender, RoutedEventArgs e)
        {
            IntPtr hwnd = new WindowInteropHelper(Application.Current.MainWindow).Handle;
            var dtm = DataTransferManagerHelper.GetForWindow(hwnd);
            dtm.DataRequested += OnDataRequested;
        }

        async void OnDataRequested(DataTransferManager sender, DataRequestedEventArgs args)
        {
            var deferral = args.Request.GetDeferral();

            try
            {
                DataPackage dp = args.Request.Data;
                dp.Properties.Title = DataPackageTitle.Text;

                switch (currentShareMode)
                {
                    case ShareMode.Text:
                        dp.SetText(DataPackageText.Text);
                        break;

                    case ShareMode.Weblink:
                        dp.SetWebLink(new System.Uri(DataPackageWeblink.Text));
                        break;

                    case ShareMode.StorageItem:
                        var filesToShare = new List<IStorageItem>();
                        StorageFile imageFile = await StorageFile.GetFileFromPathAsync(AppDomain.CurrentDomain.BaseDirectory + "Images\\image.jpg");
                        filesToShare.Add(imageFile);

                        dp.SetStorageItems(filesToShare);
                        break;
                }
            }
            finally
            {
                deferral.Complete();
            }
        }

        private void ShareTextButton_Click(object sender, RoutedEventArgs e)
        {
            currentShareMode = ShareMode.Text;
            ShowShareUI();
        }

        private void ShareWeblinkButton_Click_1(object sender, RoutedEventArgs e)
        {
            currentShareMode = ShareMode.Weblink;
            ShowShareUI();
        }

        private void ShareImageButton_Click(object sender, RoutedEventArgs e)
        {
            currentShareMode = ShareMode.StorageItem;
            ShowShareUI();
        }

        private void ShowShareUI()
        {
            IntPtr hwnd = new WindowInteropHelper(Application.Current.MainWindow).Handle;
            DataTransferManagerHelper.ShowShareUIForWindow(hwnd);
        }
    }
}
