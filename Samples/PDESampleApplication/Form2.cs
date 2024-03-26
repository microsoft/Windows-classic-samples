using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection.Emit;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Windows.Security.Cryptography;
using Windows.Security.DataProtection;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.UI.Xaml.Shapes;

namespace PDETestApp
{
    public partial class Form2 : Form
    {
        String g_selectedFolder = String.Empty;
        String g_selectedFile = String.Empty;
        public Form2()
        {
            InitializeComponent();
        }

        private void Form2_load(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
            }
            else
            {
                LogLine("Personal Data Encryption is enabled.");
                udpm.DataAvailabilityStateChanged += M_udpm_DataAvailabilityStateChanged;
                LogCurrentDataAvailability();
                LogLine("Listening to DataAvailabilityStateChanged event");
            }
        }

        private void M_udpm_DataAvailabilityStateChanged(UserDataProtectionManager sender, UserDataAvailabilityStateChangedEventArgs args)
        {
            LogLine("DataAvailabilityStateChanged event received");
            LogCurrentDataAvailability();
        }
        private void LogLine(string msg)
        {
            if (InvokeRequired)
            {
                this.Invoke(new Action<string>(LogLine), new object[] { msg });
                return;
            }
            string ts = DateTime.Now.ToString("MM/dd/yy HH:mm:ss.fff");
            textBox2.Text += "[" + ts + "] " + msg + "\r\n";
            textBox2.Select(bufferInputTextBox.TextLength, 0);
            textBox2.ScrollToCaret();
            Console.WriteLine(msg);
        }


        private void LogCurrentDataAvailability()
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
            }
            else
            {
                bool l1Avl = udpm.IsContinuedDataAvailabilityExpected(UserDataAvailability.AfterFirstUnlock);
                bool l2Avl = udpm.IsContinuedDataAvailabilityExpected(UserDataAvailability.WhileUnlocked);
                LogLine("IsContinuedDataAvailabilityExpected AfterFirstUnlock: " + l1Avl + ", WhileUnlocked: " + l2Avl);
            }
        }

        async void ProtectAndLog(IStorageItem item, UserDataAvailability level)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
            }
            else
            {
                var protectResult = await udpm.ProtectStorageItemAsync(item, UserDataAvailability.AfterFirstUnlock);
                if (protectResult == UserDataStorageItemProtectionStatus.Succeeded)
                {
                    LogLine("Protected " + item.Name + " to level " + level);
                }
                else
                {
                    LogLine("Protection failed for " + item.Name + " to level " + level + ", status: " + protectResult);
                }
            }
        }

        async void ProtectFolderRecursively(StorageFolder folder, UserDataAvailability level)
        {
            // Protect the folder first so new files / folders after this point will
            // get protected automatically.
            ProtectAndLog(folder, level);

            // Protect all sub-folders recursively.
            var subFolders = await folder.GetFoldersAsync();
            foreach (var subFolder in subFolders)
            {
                ProtectFolderRecursively(subFolder, level);
            }

            // Finally protect all existing files in the folder.
            var files = await folder.GetFilesAsync();
            foreach (var file in files)
            {
                ProtectAndLog(file, level);
            }
        }

        async void UnprotectBuffer(String protectbase64EncodedContent)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
            }
            else
            {
                var protectedBuffer = CryptographicBuffer.DecodeFromBase64String(protectbase64EncodedContent);
                try
                {
                    var result = await udpm.UnprotectBufferAsync(protectedBuffer);
                    if (result.Status == UserDataBufferUnprotectStatus.Succeeded)
                    {
                        String unprotectedText = CryptographicBuffer.ConvertBinaryToString(BinaryStringEncoding.Utf8, result.UnprotectedBuffer);
                        LogLine("Result of Unprotecting the buffer:" + unprotectedText
                            );
                        bufferOutputTextBox.Text = "";
                        bufferOutputTextBox.Text = unprotectedText;

                        LogLine("Status of Unprotectng the buffer:" + result.Status);
                    }
                    else
                    {
                        LogLine("This protected buffer is currently unavailable for unprotection");
                    }
                }
                catch(Exception ex) 
                {
                    LogLine("Please verify first the input text provided for unprotecting!");
                    LogLine(ex.ToString());
                }
            }
        }

        async void ProtectBuffer(String text, UserDataAvailability level)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
            }
            else
            {
                if (text.Length == 0)
                {
                    return;
                }
                var buffer = CryptographicBuffer.ConvertStringToBinary(text, BinaryStringEncoding.Utf8);
                var protectedContent = await udpm.ProtectBufferAsync(buffer, level);
                String protectbase64EncodedContent = CryptographicBuffer.EncodeToBase64String(protectedContent);
                bufferOutputTextBox.Text = protectbase64EncodedContent;
                LogLine("Protected buffer: " + protectbase64EncodedContent);
            }
        }

        private async void FolderL1_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            if (g_selectedFolder.Length > 0)
            {
                StorageFolder folder = await StorageFolder.GetFolderFromPathAsync(g_selectedFolder);
                this.ProtectFolderRecursively(folder, UserDataAvailability.AfterFirstUnlock);
            }
        }

        private async void FolderL2_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            if (g_selectedFolder.Length > 0)
            {
                StorageFolder folder = await StorageFolder.GetFolderFromPathAsync(g_selectedFolder);
                this.ProtectFolderRecursively(folder, UserDataAvailability.WhileUnlocked);
            }
        }

        private async void FolderUnprotect_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            if (g_selectedFolder.Length > 0)
            {
                StorageFolder folder = await StorageFolder.GetFolderFromPathAsync(g_selectedFolder);
                this.ProtectFolderRecursively(folder, UserDataAvailability.Always);
            }
        }

        private async void FileL1_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            if (g_selectedFile.Length > 0)
            {
                IStorageItem item = await StorageFile.GetFileFromPathAsync(g_selectedFile);
                this.ProtectAndLog(item, UserDataAvailability.AfterFirstUnlock);
            }
        }

        private async void FileL2_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            if (g_selectedFile.Length > 0)
            {
                IStorageItem item = await StorageFile.GetFileFromPathAsync(g_selectedFile);
                this.ProtectAndLog(item, UserDataAvailability.WhileUnlocked);
            }
        }

        private async void FileUnprotect_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            if (g_selectedFile.Length > 0)
            {
                IStorageItem item = await StorageFile.GetFileFromPathAsync(g_selectedFile);
                this.ProtectAndLog(item, UserDataAvailability.Always);
            }

        }

        private void BufferL1_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            ProtectBuffer(bufferInputTextBox.Text, UserDataAvailability.AfterFirstUnlock);
        }

        private void BufferL2_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            ProtectBuffer(bufferInputTextBox.Text, UserDataAvailability.WhileUnlocked);
        }

        private void BufferUnprotect_Click(object sender, EventArgs e)
        {
            var udpm = UserDataProtectionManager.TryGetDefault();
            if (udpm == null)
            {
                LogLine("Personal Data Encryption is not supported or enabled. Restart this app to check again.");
                return;
            }
            if (bufferInputTextBox.Text.Length > 0)
            {
                UnprotectBuffer(bufferInputTextBox.Text);
            }
        }

        private void FolderSelectBrowse_Click(object sender, EventArgs e)
        {
            if (folderBrowserDialog.ShowDialog() == DialogResult.OK)
            {
                listViewSelectedFolder.Items.Clear();
                listViewSelectedFolder.Items.Add(folderBrowserDialog.SelectedPath.Trim());
                g_selectedFolder = folderBrowserDialog.SelectedPath;
            }
        }

        private void FileSelectBrowse_Click(object sender, EventArgs e)
        {
            if (fileBrowserDialog.ShowDialog() == DialogResult.OK)
            {
                listViewSelectedFile.Items.Clear();
                listViewSelectedFile.Items.Add(fileBrowserDialog.FileName.Trim());
                g_selectedFile = fileBrowserDialog.FileName;
            }
        }
    }
}
