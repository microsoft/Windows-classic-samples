//
//  <copyright file="MainWindow.xaml.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Linq;
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
using ChatObjectModel;
using System.ComponentModel;
using System.Windows.Threading;
using System.Globalization;

namespace ChatWindow
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }
        private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.DataContext == null)
                this.DataContext = new ChatObjectModel.ObjectModel();
            ObjectModel objModel = (ObjectModel)this.DataContext;
            objModel.Connect(this.UserName.Text);
            objModel.ResponseReceived += ChatReceived;
            objModel.PropertyChanged += objModel_PropertyChanged;
            this.ConnectButton.IsEnabled = false;
            this.UserName.IsEnabled = false;
        }

        void objModel_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "Connected")
            {
                this.Dispatcher.Invoke(DispatcherPriority.Normal,
                   new Action(() => this.ConnectedChanged()));
            }
        }

        private void ConnectedChanged()
        {
            ObjectModel objModel = this.DataContext as ObjectModel;
            bool connected = objModel.Connected;

            if (connected)
            {
                this.UserName.IsReadOnly = true;
                this.ChatEntry.IsEnabled = true;
                this.Send.IsEnabled = true;
            }
            else
            {
                this.ChatEntry.IsEnabled = false;
                this.Send.IsEnabled = false;
                this.ChatText.Text = this.ChatText.Text + "<Disconnected>"
                   + Environment.NewLine;
                this.ConnectButton.IsEnabled = true;
            }
        }

        private void ChatReceived(object sender, ChatReceivedEventArgs e)
        {
            string formattedLine = String.Format(CultureInfo.CurrentCulture, "[{0}] {1}{2}",
               e.User, e.Text, Environment.NewLine);
            this.ChatText.Text = this.ChatText.Text + formattedLine;
        }

        private void Send_Click(object sender, RoutedEventArgs e)
        {
            ObjectModel objModel = this.DataContext as ObjectModel;
            objModel.SendChat(this.ChatEntry.Text);
        }
    }

}
