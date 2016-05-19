// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// -------------------------------------------------------------------- 
namespace TestLargeMessageQueue
{
    using System;
    using System.Collections.Generic;
    using System.Text;
    using System.Windows;
    using System.Windows.Controls;
    using System.Windows.Data;
    using System.Windows.Documents;
    using System.Windows.Input;
    using System.Windows.Media;
    using System.Windows.Media.Imaging;
    using System.Windows.Navigation;
    using System.Windows.Shapes;
    using System.Messaging;
    using Microsoft.Samples.MessageQueuing.LargeMessageQueue;
    using System.IO;
    using Microsoft.Win32;
    using System.Globalization;

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        // Handle for the send/receive queue
        MessageQueue queue;

        public Window1()
        {
            InitializeComponent();

            // Initializing all the required values
            progressBarSend.Visibility = Visibility.Hidden;
            progressBarReceive.Visibility = Visibility.Hidden;

            textFileName.Clear();
            textFileName.Text = Directory.GetCurrentDirectory() + @"\" + "default.bmp";

            // Set the fragment size to 10KB
            textFragmentSize.Clear();
            textFragmentSize.Text = "10000";

            // The queue can be public or private queue
            String queuePath = @".\private$\largemessageq";

            // Must be same as Parameters.STATUS_QUEUE in the LargeMessageQueue project
            String statusQueuePath = @".\private$\statusq";

            // Flag to create transactional queue
            bool transactional = true;

            // Delete and create both the queues
            try
            {
                MessageQueue.Delete(queuePath);
            }
            catch (MessageQueueException)
            {
                // Don't do anything. Queue already not present
            }

            try
            {
                MessageQueue.Delete(statusQueuePath);
            }
            catch (MessageQueueException)
            {
                // Don't do anything. Queue already not present
            }

            try
            {
                queue = MessageQueue.Create(queuePath, transactional);
                MessageQueue.Create(statusQueuePath, transactional);
            }
            catch (MessageQueueException mqe)
            {
                MessageBox.Show("Error: " + mqe.Message, "Error!");
            }

            this.SetQueueViewerStatus();
        }

        // Reinitializes the objects being used
        private void labelClearObject_MouseDown(object sender, MouseButtonEventArgs e)
        {
            textFileName.Clear();
            textFileName.BorderBrush = Brushes.LightBlue;
            imageSendFile.Visibility = Visibility.Hidden;
            imageReceiveFile.Visibility = Visibility.Hidden;
            progressBarSend.Visibility = Visibility.Hidden;
            progressBarReceive.Visibility = Visibility.Hidden;
            labelFileSize.Background = Brushes.Transparent;
            this.SetQueueViewerStatus();
        }

        // Clears the fragment size text box
        private void labelClearFragment_MouseDown(object sender, MouseButtonEventArgs e)
        {
            textFragmentSize.Clear();
            textFileName.BorderBrush = Brushes.LightBlue;
        }

        // Use normal Send
        private void buttonSendNormal_Click(object sender, RoutedEventArgs e)
        {
            this.SendGeneric(false);
        }

        // Use large message sample API
        private void buttonSend_Click(object sender, RoutedEventArgs e)
        {
            this.SendGeneric(true);
        }

        // Do lossy receive. The first message will be destructively received and thus holes in the large message sequence is simulated
        private void buttonLossyReceive_Click(object sender, RoutedEventArgs e)
        {
            this.ReceiveGeneric(true);
        }

        // Do complete receive without simulating holes
        private void buttonReceive_Click(object sender, RoutedEventArgs e)
        {
            this.ReceiveGeneric(false);
        }

        // Open file dialog operation
        private void buttonFile_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.InitialDirectory = "c:\\";
            openFileDialog.Filter = "BMP Image files (*.bmp)|*.bmp";
            openFileDialog.FilterIndex = 1;
            openFileDialog.RestoreDirectory = true;
            Nullable<bool> dialogResult = openFileDialog.ShowDialog();

            if (dialogResult == true)
            {
                textFileName.Text = openFileDialog.FileName;
            }
        }

        // Populate sender side image viewer and file size (less than or greater than 4MB) indicator
        private void textFileName_TextChanged(object sender, TextChangedEventArgs e)
        {
            // Check the validity of the file name entered in the text box as it is typed
            if (this.IsValidBmpFile(textFileName.Text))
            {
                FileInfo fileInfo = new FileInfo(textFileName.Text);
                float size = (float)(fileInfo.Length) / 1000000;
                labelFileSize.Content = size.ToString() + "MB";
                if (fileInfo.Length > 4000000)
                {
                    labelFileSize.Background = Brushes.LightBlue;
                }
                else
                {
                    labelFileSize.Background = Brushes.LightGreen;
                }

                BitmapImage bmpImage = new BitmapImage();
                bmpImage.BeginInit();
                bmpImage.UriSource = new Uri(textFileName.Text);
                bmpImage.EndInit();
                imageSendFile.Source = bmpImage;

                textFileName.BorderBrush = Brushes.Green;
                imageSendFile.Visibility = Visibility.Visible;
            }
            else
            {
                textFileName.BorderBrush = Brushes.Red;
                imageReceiveFile.Visibility = Visibility.Hidden;

                labelFileSize.Content = string.Empty;
                labelFileSize.Background = Brushes.Transparent;
            }
        }

        // Handles both normal and large message send operations
        private void SendGeneric(bool useLargeMessageSample)
        {
            progressBarReceive.Visibility = Visibility.Hidden;
            Utilities.StartProgressBar(progressBarSend);
            try
            {
                MessageQueueTransaction transaction = new MessageQueueTransaction();
                transaction.Begin();

                IMessageFormatter formatter = new BinaryMessageFormatter();
                Message message = new Message();

                FileInfo fileInfo = new FileInfo(textFileName.Text);
                FileStream fStream = fileInfo.OpenRead();
                byte[] b = new byte[fStream.Length];
                fStream.Read(b, 0, b.Length);
                MemoryStream mStream = new MemoryStream(b, 0, b.Length, false);

                message.Label = fileInfo.Name;
                message.Formatter = formatter;
                message.UseDeadLetterQueue = true;
                message.Body = mStream;

                int fragmentSize = 0;
                if (textFragmentSize.Text.Length != 0)
                {
                    fragmentSize = Int32.Parse(textFragmentSize.Text, CultureInfo.InvariantCulture);
                }

                if (useLargeMessageSample)
                {
                    LargeMessageQueue largeMessageQueue = new LargeMessageQueue(this.queue, fragmentSize);
                    largeMessageQueue.Send(message, transaction);
                }
                else
                {
                    this.queue.Send(message, transaction);
                }

                transaction.Commit();

                Utilities.StopProgressBar(progressBarSend);
                this.SetQueueViewerStatus();

                MessageBox.Show("Send successful!", "Success!");
            }
            catch (Exception ex)
            {
                Utilities.ErrorProgressBar(progressBarSend);
                this.SetQueueViewerStatus();

                MessageBox.Show("Send Error: " + ex.Message, "Error!");
            }
        }

        // Handles both lossy and non-lossy large message receive operations
        private void ReceiveGeneric(bool isLossy)
        {
            progressBarSend.Visibility = Visibility.Hidden;
            Utilities.StartProgressBar(progressBarReceive);
            try
            {
                MessageQueueTransaction transaction = new MessageQueueTransaction();
                transaction.Begin();

                if (isLossy)
                {
                    this.queue.Receive(new TimeSpan(0, 0, 10));
                }

                LargeMessageQueue largeMessageQueue = new LargeMessageQueue(this.queue, 0);
                System.Messaging.Message message = largeMessageQueue.Receive(new TimeSpan(0, 0, 10), transaction);

                // Redundant. Will never execute these for lossy receive
                message.Formatter = new BinaryMessageFormatter();
                MemoryStream mStream = (MemoryStream)message.Body;

                string newLabel = "Received - " + message.Label;
                FileStream fStream = new FileStream(newLabel, FileMode.Create, FileAccess.ReadWrite);
                mStream.WriteTo(fStream);
                mStream.Close();
                fStream.Close();

                transaction.Commit();

                this.LoadImageFile(imageReceiveFile, newLabel);
                imageReceiveFile.Visibility = Visibility.Visible;
                Utilities.StopProgressBar(progressBarReceive);
                this.SetQueueViewerStatus();

                MessageBox.Show("Receive successful!", "Success!");
            }
            catch (LargeMessageQueueException lmqe)
            {
                Utilities.ErrorProgressBar(progressBarReceive);
                imageReceiveFile.Visibility = Visibility.Hidden;
                this.SetQueueViewerStatus();

                MessageBox.Show("Receive Error: " + lmqe.Message + "\n Large Sequence Id:" + lmqe.CorrelationId, "Error!");
            }
            catch (Exception ex)
            {
                Utilities.ErrorProgressBar(progressBarReceive);
                imageReceiveFile.Visibility = Visibility.Hidden;
                this.SetQueueViewerStatus();

                MessageBox.Show("Receive Error: " + ex.Message, "Error!");
            }
        }

        // Handles Drop Event on the window for files.
        private void DropFile(object sender, DragEventArgs e)
        {
            textFileName.Clear();

            string[] fileNames = e.Data.GetData(DataFormats.FileDrop, true) as string[];
            string fileName = fileNames[0];
            try
            {
                FileInfo fileInfo = new FileInfo(fileName);
                textFileName.Text = fileName;
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: Could not read file: " + ex.Message);
            }

            // Mark the event as handled so that the default handler is not used
            e.Handled = true;
        }

        // Get the message count in the queue
        private int GetMessageCount()
        {
            List<Message> messageList = new List<Message>();

            try
            {
                // there is no messagequeue.count or .length
                // this is pretty much the only reliable (and ugly) way of getting
                // the number of messages in a queue
                Message message = this.queue.Peek(TimeSpan.Zero);
                while (true)
                {
                    if (message == null)
                    {
                        break;
                    }

                    messageList.Add(message);
                    message = this.queue.PeekByLookupId(MessageLookupAction.Next, message.LookupId);
                }
            }
            catch (MessageQueueException)
            {
                // Don't do anything
            }
            catch (InvalidOperationException)
            {
                // Don't do anything
            }

            return messageList.Count;
        }

        // Loads the bmp image into the image viewer
        private void LoadImageFile(Image image, string label)
        {
            string fileName = Directory.GetCurrentDirectory() + @"\" + label;
            if (this.IsValidBmpFile(fileName))
            {
                BitmapImage bmpImage = new BitmapImage();
                bmpImage.BeginInit();
                bmpImage.UriSource = new Uri(fileName);
                bmpImage.EndInit();
                image.Source = bmpImage;
            }
            else
            {
                image.Visibility = Visibility.Hidden;
            }
        }

        // Check if the file selected is a valid bmp file
        private bool IsValidBmpFile(string fileName)
        {
            try
            {
                FileInfo fileInfo = new FileInfo(fileName);
                if (fileInfo.Exists)
                {
                    if (string.Compare(fileInfo.Extension, 0, ".BMP", 0, fileInfo.Extension.Length, StringComparison.OrdinalIgnoreCase) == 0)
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
            catch (Exception)
            {
                return false;
            }
        }

        // Changes the color and content of the queue viewer
        private void SetQueueViewerStatus()
        {
            int queueMessageCount = this.GetMessageCount();
            if (queueMessageCount != 0)
            {
                labelQueueViewer.Background = Brushes.LightGreen;
                labelQueueViewer.Content = queueMessageCount + " messages";
            }
            else
            {
                labelQueueViewer.Background = Brushes.PaleVioletRed;
                labelQueueViewer.Content = "Empty";
            }
        }
    }
}
