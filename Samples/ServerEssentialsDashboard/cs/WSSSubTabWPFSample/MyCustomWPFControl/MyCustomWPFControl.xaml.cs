//
//  <copyright file="MyCustomWPFControl.xaml.cs" company="Microsoft">
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

namespace WSSSubTabWPFSample
{
    /// <summary>
    /// Interaction logic for MyCustomWPFControl.xaml
    /// </summary>
    public partial class MyCustomWPFControl : UserControl
    {
        public MyCustomWPFControl()
        {
            InitializeComponent();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Welcome to WPF addin for Dashboard!");
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Welcome to WPF addin for Dashboard!");
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            MessageBox.Show("Welcome to WPF addin for Dashboard!");
        }
    }
}
