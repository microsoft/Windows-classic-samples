// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: TIPAutoClaims.cs
//  TIPCustomization Sample Application
//
//  This sample is an enhanced version of the AutoClaims sample.
//  In addition to allowing the user to enter vehicle information
//  and illustrating car damage using Inking controls, this sample
//  demostrates how to control the layout and input mode for the 
//  Text Input Panel.  
//
//  The features used are: 
//   - TextInputPanel (Customizing TIP's behavior)
//   - InkEdit (providing Correction UI)
//   - InkPicture (demonstrating multiple layers of ink within a control)
//
//--------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Windows.Forms;
using System.Data;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;
// The Text Input Panel (TIP) namespace, which contains the TIP API.
using Microsoft.Ink.TextInput;

namespace TIPAutoClaims
{
    /// <summary>
    /// InkLayer represents a given ink layer, which has a
    /// color and visibility setting associated with it.
    /// </summary>
    struct InkLayer
    {
        // Declare the ink, color, and visibility for this layer
        private Ink myInk;
        private Color myColor;
        private bool myHidden;

        /// <summary>
        /// Constructor for InkLayer
        /// </summary>
        /// <param name="ink">The ink</param>
        /// <param name="color">The color of the layer</param>
        /// <param name="hidden">True if the layer is hidden</param>
        public InkLayer(Ink ink, Color color, bool hidden)
        {
            myInk = ink;
            myColor = color;
            myHidden = hidden;
        }

        /// <summary>
        /// The property used to set the ink associated with this layer
        /// </summary>
        public Ink ActiveInk
        {
            get 
            {
                return myInk;
            }
            set
            {
                myInk = value;
            }
        }

        /// <summary>
        /// The color associated with this layer
        /// </summary>
        public Color ActiveColor
        {
            get
            {
                return myColor;
            }
            set
            {
                myColor = value;
            }
        }

        /// <summary>
        /// True if this layer is hidden.
        /// </summary>
        public bool Hidden
        {
            get
            {
                return myHidden;
            }
            set
            {
                myHidden = value;
            }
        }
    }

    /// <summary>
    /// The AutoClaims Sample Application form class
    /// </summary>
    public class AutoClaims : System.Windows.Forms.Form
    {
        #region Class Members
        // You can receive a REGDB_E_CLASSNOTREG if your system doesn't have 
        // the neccessary components installed to create the TextInputPanel object.  
        // Win32 Error code for REGDB_E_CLASSNOTREG
        private readonly int ObjectsNotInstalledError = -2147221164; // 0x80040154

        // Declare the array of ink layers used the vehicle illustration.
        InkLayer[] inkLayers;

        // Declare an empty ink object to be used when we are not drawing ink because 
        // a layer is hidden. 
        Ink emptyInk = null;

        // Array for storing ink input.
        System.Collections.Generic.List<Ink> insertedInk;

        // Declare InkOverlay to draw ink.
        InkOverlay theInkOverlay;
        
        // Declare TextInputPanel objects (used to collect handwriting 
        // for the edit controls on the form).
        // tipShared is shared between the Year, Make, and Model fields.
        TextInputPanel tipShared = null;

        // TipPolicyNumber is used for the Policy Number field.
        TextInputPanel tipPolicyNumber = null;

        // TipName is used for the Policy Name field.
        TextInputPanel tipName = null;

        // TipSignature is used for the Signature field.
        TextInputPanel tipSignature = null;

        // Declare a value to hold the index of selected ink.
        int selectedIndex = -1;

        // Declare a value to hold whether the selected ink is hidden.
        bool selectedHidden = false;

        // seForm is used to show TIP events.
        ShowEvents seForm;
        #endregion

        #region Standard Template Code
        internal System.Windows.Forms.CheckBox chHideLayer;
        internal System.Windows.Forms.Label lblVehicleDamage;
        internal System.Windows.Forms.Label lblVehicleDescription;
        internal System.Windows.Forms.Label lblModel;
        internal System.Windows.Forms.Label lblYear;
        internal System.Windows.Forms.Label lblMake;
        internal System.Windows.Forms.Label lblPolicyNumber;
        internal System.Windows.Forms.Label lblInsuredName;
        internal System.Windows.Forms.Label lblInstructions;
        internal System.Windows.Forms.Label lblTitle;
        internal System.Windows.Forms.PictureBox pictVehicleBox;
        internal System.Windows.Forms.PictureBox pictPolicyBox;
        internal System.Windows.Forms.PictureBox pictHorzontalBar2;
        internal System.Windows.Forms.PictureBox pictHorzontalBar3;
        internal System.Windows.Forms.PictureBox pictHorzontalBar1;
        internal System.Windows.Forms.PictureBox pictCheckmarks;
        internal System.Windows.Forms.ListBox lstAnnotationLayer;
        private Microsoft.Ink.InkPicture inkPictVehicle;
        private Microsoft.Ink.InkEdit inkEdPolicyNumber;
        private Microsoft.Ink.InkEdit inkEdName;
        private Microsoft.Ink.InkEdit inkEdYear;
        private Microsoft.Ink.InkEdit inkEdMake;
        private Microsoft.Ink.InkEdit inkEdModel;
        private Button ShowEvent;
        private Panel panelForSignature;
        private InkEdit inkEdSignature;
        private Label label1;
        private System.ComponentModel.Container components = null;
        #endregion
        
        public AutoClaims()
        {
            #region Standard Template Code
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
            #endregion

            // Instantiate the form that displays TIP event information
            seForm = new ShowEvents();
        }

        #region Standard Template Code
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose( bool disposing )
        {
            if( disposing )
            {
                if (components != null) 
                {
                    components.Dispose();
                }
                
                // dispose any resources associated with our input panels
                if (tipShared != null)
                {
                    tipShared.Dispose();
                }
                if (tipPolicyNumber != null)
                {
                    tipPolicyNumber.Dispose();
                }
                if (tipName != null)
                {
                    tipName.Dispose();
                }

                if (tipSignature != null)
                {
                    tipSignature.Dispose();
                }

                // dispose any resources associated with our ink objects
                foreach (InkLayer layer in inkLayers)
                {
                    layer.ActiveInk.Dispose();
                }
                
                // Dispose the empty ink object
                if(emptyInk != null)
                {
                    emptyInk.Dispose();
                }
            }
            base.Dispose( disposing );
        }
        #endregion

        #region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AutoClaims));
            this.lblVehicleDamage = new System.Windows.Forms.Label();
            this.lblVehicleDescription = new System.Windows.Forms.Label();
            this.lblModel = new System.Windows.Forms.Label();
            this.lblYear = new System.Windows.Forms.Label();
            this.lblMake = new System.Windows.Forms.Label();
            this.lblPolicyNumber = new System.Windows.Forms.Label();
            this.lblInsuredName = new System.Windows.Forms.Label();
            this.chHideLayer = new System.Windows.Forms.CheckBox();
            this.pictHorzontalBar2 = new System.Windows.Forms.PictureBox();
            this.pictVehicleBox = new System.Windows.Forms.PictureBox();
            this.pictHorzontalBar3 = new System.Windows.Forms.PictureBox();
            this.pictPolicyBox = new System.Windows.Forms.PictureBox();
            this.pictHorzontalBar1 = new System.Windows.Forms.PictureBox();
            this.lblInstructions = new System.Windows.Forms.Label();
            this.lblTitle = new System.Windows.Forms.Label();
            this.pictCheckmarks = new System.Windows.Forms.PictureBox();
            this.inkPictVehicle = new Microsoft.Ink.InkPicture();
            this.lstAnnotationLayer = new System.Windows.Forms.ListBox();
            this.inkEdPolicyNumber = new Microsoft.Ink.InkEdit();
            this.inkEdName = new Microsoft.Ink.InkEdit();
            this.inkEdYear = new Microsoft.Ink.InkEdit();
            this.inkEdMake = new Microsoft.Ink.InkEdit();
            this.inkEdModel = new Microsoft.Ink.InkEdit();
            this.ShowEvent = new System.Windows.Forms.Button();
            this.panelForSignature = new System.Windows.Forms.Panel();
            this.inkEdSignature = new Microsoft.Ink.InkEdit();
            this.label1 = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.pictHorzontalBar2)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictVehicleBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictHorzontalBar3)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictPolicyBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictHorzontalBar1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictCheckmarks)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.inkPictVehicle)).BeginInit();
            this.SuspendLayout();
            // 
            // lblVehicleDamage
            // 
            this.lblVehicleDamage.AutoSize = true;
            this.lblVehicleDamage.BackColor = System.Drawing.Color.White;
            this.lblVehicleDamage.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblVehicleDamage.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblVehicleDamage.Location = new System.Drawing.Point(8, 408);
            this.lblVehicleDamage.Name = "lblVehicleDamage";
            this.lblVehicleDamage.Size = new System.Drawing.Size(256, 25);
            this.lblVehicleDamage.TabIndex = 58;
            this.lblVehicleDamage.Text = "Description of vehicle damage";
            this.lblVehicleDamage.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblVehicleDamage.UseCompatibleTextRendering = true;
            // 
            // lblVehicleDescription
            // 
            this.lblVehicleDescription.AutoSize = true;
            this.lblVehicleDescription.BackColor = System.Drawing.Color.White;
            this.lblVehicleDescription.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblVehicleDescription.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblVehicleDescription.Location = new System.Drawing.Point(8, 240);
            this.lblVehicleDescription.Name = "lblVehicleDescription";
            this.lblVehicleDescription.Size = new System.Drawing.Size(386, 25);
            this.lblVehicleDescription.TabIndex = 56;
            this.lblVehicleDescription.Text = "Enter the involved vehicle listed on the policy ";
            this.lblVehicleDescription.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblVehicleDescription.UseCompatibleTextRendering = true;
            // 
            // lblModel
            // 
            this.lblModel.AutoSize = true;
            this.lblModel.BackColor = System.Drawing.Color.Gainsboro;
            this.lblModel.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblModel.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblModel.Location = new System.Drawing.Point(12, 352);
            this.lblModel.Name = "lblModel";
            this.lblModel.Size = new System.Drawing.Size(73, 29);
            this.lblModel.TabIndex = 52;
            this.lblModel.Text = "Model:";
            this.lblModel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblModel.UseCompatibleTextRendering = true;
            // 
            // lblYear
            // 
            this.lblYear.AutoSize = true;
            this.lblYear.BackColor = System.Drawing.Color.Gainsboro;
            this.lblYear.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblYear.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblYear.Location = new System.Drawing.Point(12, 275);
            this.lblYear.Name = "lblYear";
            this.lblYear.Size = new System.Drawing.Size(135, 29);
            this.lblYear.TabIndex = 51;
            this.lblYear.Text = "Year (YYYY):";
            this.lblYear.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblYear.UseCompatibleTextRendering = true;
            // 
            // lblMake
            // 
            this.lblMake.AutoSize = true;
            this.lblMake.BackColor = System.Drawing.Color.Gainsboro;
            this.lblMake.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblMake.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblMake.Location = new System.Drawing.Point(12, 312);
            this.lblMake.Name = "lblMake";
            this.lblMake.Size = new System.Drawing.Size(67, 29);
            this.lblMake.TabIndex = 50;
            this.lblMake.Text = "Make:";
            this.lblMake.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblMake.UseCompatibleTextRendering = true;
            // 
            // lblPolicyNumber
            // 
            this.lblPolicyNumber.AutoSize = true;
            this.lblPolicyNumber.BackColor = System.Drawing.Color.Gainsboro;
            this.lblPolicyNumber.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblPolicyNumber.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblPolicyNumber.Location = new System.Drawing.Point(11, 136);
            this.lblPolicyNumber.Name = "lblPolicyNumber";
            this.lblPolicyNumber.Size = new System.Drawing.Size(157, 29);
            this.lblPolicyNumber.TabIndex = 47;
            this.lblPolicyNumber.Text = "Policy Number:";
            this.lblPolicyNumber.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblPolicyNumber.UseCompatibleTextRendering = true;
            // 
            // lblInsuredName
            // 
            this.lblInsuredName.AutoSize = true;
            this.lblInsuredName.BackColor = System.Drawing.Color.Gainsboro;
            this.lblInsuredName.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblInsuredName.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblInsuredName.Location = new System.Drawing.Point(11, 168);
            this.lblInsuredName.Name = "lblInsuredName";
            this.lblInsuredName.Size = new System.Drawing.Size(154, 29);
            this.lblInsuredName.TabIndex = 46;
            this.lblInsuredName.Text = "Insured Name:";
            this.lblInsuredName.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblInsuredName.UseCompatibleTextRendering = true;
            // 
            // chHideLayer
            // 
            this.chHideLayer.CheckAlign = System.Drawing.ContentAlignment.TopLeft;
            this.chHideLayer.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.chHideLayer.Location = new System.Drawing.Point(472, 432);
            this.chHideLayer.Name = "chHideLayer";
            this.chHideLayer.Size = new System.Drawing.Size(94, 65);
            this.chHideLayer.TabIndex = 61;
            this.chHideLayer.Text = "Hide Selected Layer";
            this.chHideLayer.UseCompatibleTextRendering = true;
            this.chHideLayer.CheckedChanged += new System.EventHandler(this.chHideLayer_CheckedChanged);
            // 
            // pictHorzontalBar2
            // 
            this.pictHorzontalBar2.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.pictHorzontalBar2.Location = new System.Drawing.Point(8, 224);
            this.pictHorzontalBar2.Name = "pictHorzontalBar2";
            this.pictHorzontalBar2.Size = new System.Drawing.Size(536, 2);
            this.pictHorzontalBar2.TabIndex = 57;
            this.pictHorzontalBar2.TabStop = false;
            // 
            // pictVehicleBox
            // 
            this.pictVehicleBox.BackColor = System.Drawing.Color.Gainsboro;
            this.pictVehicleBox.Location = new System.Drawing.Point(8, 272);
            this.pictVehicleBox.Name = "pictVehicleBox";
            this.pictVehicleBox.Size = new System.Drawing.Size(194, 120);
            this.pictVehicleBox.TabIndex = 49;
            this.pictVehicleBox.TabStop = false;
            // 
            // pictHorzontalBar3
            // 
            this.pictHorzontalBar3.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.pictHorzontalBar3.Location = new System.Drawing.Point(8, 400);
            this.pictHorzontalBar3.Name = "pictHorzontalBar3";
            this.pictHorzontalBar3.Size = new System.Drawing.Size(536, 2);
            this.pictHorzontalBar3.TabIndex = 48;
            this.pictHorzontalBar3.TabStop = false;
            // 
            // pictPolicyBox
            // 
            this.pictPolicyBox.BackColor = System.Drawing.Color.Gainsboro;
            this.pictPolicyBox.Location = new System.Drawing.Point(8, 128);
            this.pictPolicyBox.Name = "pictPolicyBox";
            this.pictPolicyBox.Size = new System.Drawing.Size(194, 80);
            this.pictPolicyBox.TabIndex = 44;
            this.pictPolicyBox.TabStop = false;
            // 
            // pictHorzontalBar1
            // 
            this.pictHorzontalBar1.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.pictHorzontalBar1.Location = new System.Drawing.Point(8, 113);
            this.pictHorzontalBar1.Name = "pictHorzontalBar1";
            this.pictHorzontalBar1.Size = new System.Drawing.Size(536, 2);
            this.pictHorzontalBar1.TabIndex = 43;
            this.pictHorzontalBar1.TabStop = false;
            // 
            // lblInstructions
            // 
            this.lblInstructions.Font = new System.Drawing.Font("Tahoma", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblInstructions.Location = new System.Drawing.Point(96, 44);
            this.lblInstructions.Name = "lblInstructions";
            this.lblInstructions.Size = new System.Drawing.Size(448, 65);
            this.lblInstructions.TabIndex = 42;
            this.lblInstructions.Text = "Enter the required information for the policy applicable to the auto claim you ar" +
                "e submitting. Enter the vehicle exactly as it appears on the policy or insurance" +
                " card.";
            this.lblInstructions.UseCompatibleTextRendering = true;
            // 
            // lblTitle
            // 
            this.lblTitle.AutoSize = true;
            this.lblTitle.Font = new System.Drawing.Font("Tahoma", 21.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblTitle.Location = new System.Drawing.Point(8, 8);
            this.lblTitle.Name = "lblTitle";
            this.lblTitle.Size = new System.Drawing.Size(174, 42);
            this.lblTitle.TabIndex = 41;
            this.lblTitle.Text = "Auto Claim";
            this.lblTitle.UseCompatibleTextRendering = true;
            // 
            // pictCheckmarks
            // 
            this.pictCheckmarks.BackColor = System.Drawing.Color.White;
            this.pictCheckmarks.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("pictCheckmarks.BackgroundImage")));
            this.pictCheckmarks.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.pictCheckmarks.Location = new System.Drawing.Point(33, 44);
            this.pictCheckmarks.Name = "pictCheckmarks";
            this.pictCheckmarks.Size = new System.Drawing.Size(37, 62);
            this.pictCheckmarks.TabIndex = 66;
            this.pictCheckmarks.TabStop = false;
            // 
            // inkPictVehicle
            // 
            this.inkPictVehicle.AutoRedraw = false;
            this.inkPictVehicle.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.inkPictVehicle.Image = ((System.Drawing.Image)(resources.GetObject("inkPictVehicle.Image")));
            this.inkPictVehicle.Location = new System.Drawing.Point(8, 432);
            this.inkPictVehicle.MarginX = -1;
            this.inkPictVehicle.MarginY = -1;
            this.inkPictVehicle.Name = "inkPictVehicle";
            this.inkPictVehicle.Size = new System.Drawing.Size(352, 218);
            this.inkPictVehicle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.inkPictVehicle.TabIndex = 67;
            this.inkPictVehicle.Paint += new System.Windows.Forms.PaintEventHandler(this.inkPictVehicle_Paint);
            // 
            // lstAnnotationLayer
            // 
            this.lstAnnotationLayer.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lstAnnotationLayer.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lstAnnotationLayer.ItemHeight = 19;
            this.lstAnnotationLayer.Items.AddRange(new object[] {
            "Body",
            "Windows",
            "Tires",
            "Headlights"});
            this.lstAnnotationLayer.Location = new System.Drawing.Point(376, 432);
            this.lstAnnotationLayer.Name = "lstAnnotationLayer";
            this.lstAnnotationLayer.Size = new System.Drawing.Size(90, 97);
            this.lstAnnotationLayer.TabIndex = 68;
            this.lstAnnotationLayer.SelectedIndexChanged += new System.EventHandler(this.lstAnnotationLayer_SelectedIndexChanged);
            // 
            // inkEdPolicyNumber
            // 
            this.inkEdPolicyNumber.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdPolicyNumber.InkMode = Microsoft.Ink.InkMode.Disabled;
            this.inkEdPolicyNumber.Location = new System.Drawing.Point(208, 136);
            this.inkEdPolicyNumber.Name = "inkEdPolicyNumber";
            this.inkEdPolicyNumber.Size = new System.Drawing.Size(312, 32);
            this.inkEdPolicyNumber.TabIndex = 69;
            this.inkEdPolicyNumber.Text = "";
            this.inkEdPolicyNumber.Enter += new System.EventHandler(this.inkEdPolicyNumber_Enter);
            // 
            // inkEdName
            // 
            this.inkEdName.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdName.InkMode = Microsoft.Ink.InkMode.Disabled;
            this.inkEdName.Location = new System.Drawing.Point(208, 176);
            this.inkEdName.Name = "inkEdName";
            this.inkEdName.Size = new System.Drawing.Size(312, 32);
            this.inkEdName.TabIndex = 70;
            this.inkEdName.Text = "";
            // 
            // inkEdYear
            // 
            this.inkEdYear.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdYear.InkMode = Microsoft.Ink.InkMode.Disabled;
            this.inkEdYear.Location = new System.Drawing.Point(208, 280);
            this.inkEdYear.Name = "inkEdYear";
            this.inkEdYear.Size = new System.Drawing.Size(312, 32);
            this.inkEdYear.TabIndex = 71;
            this.inkEdYear.Text = "";
            this.inkEdYear.Enter += new System.EventHandler(this.inkEdYear_Enter);
            // 
            // inkEdMake
            // 
            this.inkEdMake.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdMake.InkMode = Microsoft.Ink.InkMode.Disabled;
            this.inkEdMake.Location = new System.Drawing.Point(208, 320);
            this.inkEdMake.Name = "inkEdMake";
            this.inkEdMake.Size = new System.Drawing.Size(312, 32);
            this.inkEdMake.TabIndex = 72;
            this.inkEdMake.Text = "";
            this.inkEdMake.Enter += new System.EventHandler(this.inkEdMake_Enter);
            // 
            // inkEdModel
            // 
            this.inkEdModel.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdModel.InkMode = Microsoft.Ink.InkMode.Disabled;
            this.inkEdModel.Location = new System.Drawing.Point(208, 360);
            this.inkEdModel.Name = "inkEdModel";
            this.inkEdModel.Size = new System.Drawing.Size(312, 32);
            this.inkEdModel.TabIndex = 73;
            this.inkEdModel.Text = "";
            this.inkEdModel.Enter += new System.EventHandler(this.inkEdModel_Enter);
            // 
            // ShowEvent
            // 
            this.ShowEvent.Location = new System.Drawing.Point(397, 240);
            this.ShowEvent.Name = "ShowEvent";
            this.ShowEvent.Size = new System.Drawing.Size(123, 25);
            this.ShowEvent.TabIndex = 75;
            this.ShowEvent.Text = "Show TIP Events";
            this.ShowEvent.UseCompatibleTextRendering = true;
            this.ShowEvent.UseVisualStyleBackColor = true;
            this.ShowEvent.Click += new System.EventHandler(this.showEventButton_Click);
            // 
            // panelForSignature
            // 
            this.panelForSignature.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panelForSignature.Enabled = false;
            this.panelForSignature.Location = new System.Drawing.Point(376, 608);
            this.panelForSignature.Name = "panelForSignature";
            this.panelForSignature.Size = new System.Drawing.Size(168, 42);
            this.panelForSignature.TabIndex = 76;
            this.panelForSignature.Paint += new System.Windows.Forms.PaintEventHandler(this.panelForSignature_Paint);
            // 
            // inkEdSignature
            // 
            this.inkEdSignature.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdSignature.InkMode = Microsoft.Ink.InkMode.Disabled;
            this.inkEdSignature.Location = new System.Drawing.Point(376, 578);
            this.inkEdSignature.Name = "inkEdSignature";
            this.inkEdSignature.Size = new System.Drawing.Size(168, 26);
            this.inkEdSignature.TabIndex = 77;
            this.inkEdSignature.Text = "";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 10.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(373, 553);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(77, 17);
            this.label1.TabIndex = 78;
            this.label1.Text = "Signature :";
            // 
            // AutoClaims
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Inherit;
            this.AutoScroll = true;
            this.AutoSize = true;
            this.BackColor = System.Drawing.Color.White;
            this.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.ClientSize = new System.Drawing.Size(602, 676);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.inkEdSignature);
            this.Controls.Add(this.panelForSignature);
            this.Controls.Add(this.ShowEvent);
            this.Controls.Add(this.inkEdModel);
            this.Controls.Add(this.inkEdMake);
            this.Controls.Add(this.inkEdYear);
            this.Controls.Add(this.inkEdName);
            this.Controls.Add(this.inkEdPolicyNumber);
            this.Controls.Add(this.lstAnnotationLayer);
            this.Controls.Add(this.inkPictVehicle);
            this.Controls.Add(this.lblVehicleDamage);
            this.Controls.Add(this.lblVehicleDescription);
            this.Controls.Add(this.lblModel);
            this.Controls.Add(this.lblYear);
            this.Controls.Add(this.lblMake);
            this.Controls.Add(this.lblPolicyNumber);
            this.Controls.Add(this.lblInsuredName);
            this.Controls.Add(this.chHideLayer);
            this.Controls.Add(this.pictHorzontalBar2);
            this.Controls.Add(this.pictVehicleBox);
            this.Controls.Add(this.pictHorzontalBar3);
            this.Controls.Add(this.pictPolicyBox);
            this.Controls.Add(this.pictHorzontalBar1);
            this.Controls.Add(this.lblInstructions);
            this.Controls.Add(this.lblTitle);
            this.Controls.Add(this.pictCheckmarks);
            this.Name = "AutoClaims";
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.Text = "TIPAutoClaims";
            this.TopMost = true;
            this.Load += new System.EventHandler(this.AutoClaims_Load);
            ((System.ComponentModel.ISupportInitialize)(this.pictHorzontalBar2)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictVehicleBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictHorzontalBar3)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictPolicyBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictHorzontalBar1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.pictCheckmarks)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.inkPictVehicle)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        #endregion

        #region Standard Template Code
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() 
        {
            Application.Run(new AutoClaims());
        }
        #endregion

        /// <summary>
        /// This method occurs when the form is loaded.  It
        /// configures the insertion mode, the ink used by
        /// this application, and the default behavior for the PenInputPanels.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void AutoClaims_Load(object sender, System.EventArgs e)
        {
            // Initialize the empty ink.
            emptyInk = new Ink();
            
            // Create a list to store the Ink collected from the TIP.
            insertedInk = new System.Collections.Generic.List<Ink>();
                        
            // Create InkOverlay to display signature
            theInkOverlay = new InkOverlay(panelForSignature.Handle);
            theInkOverlay.Enabled = true;

            // Initialize the TextInputPanel objects inside a Try/Catch block
            // since the objects are not supported on some SKUs.
            try 
            {
                // Initialize the TextInputPanel objects.
                // - tipShared is attached dynamically at run-time
                // - tipPolicyNumber is only attached to the Policy Number field
                // - tipName is only attached to the Insured Name field
                tipShared = new TextInputPanel();
                tipPolicyNumber = new TextInputPanel(inkEdPolicyNumber);
                tipName = new TextInputPanel(inkEdName);
                tipSignature = new TextInputPanel(inkEdSignature);

                // Policy numbers are typically prone to recognizer errors, so we'll 
                // default to the keyboard panel to be visible for the Policy number.
                // We set the PreferredInPlaceDirection to Top so that TIP does not cover the control below
                // We also set default inplace state to be expanded to save a step for the user.
                tipPolicyNumber.DefaultInputArea = PanelInputArea.Keyboard;
                tipPolicyNumber.PreferredInPlaceDirection = InPlaceDirection.Top;
                tipPolicyNumber.DefaultInPlaceState = InPlaceState.Expanded;


                // Policy name are also prone to recognizer error, so we'll default
                // to Character Pad.
                // We set the PreferredInPlaceDirection to Top so that TIP does not cover the control below
                // We also set default inplace state to be expanded.
                tipName.DefaultInputArea = PanelInputArea.WritingPad;
                tipName.PreferredInPlaceDirection = InPlaceDirection.Top;
                tipName.DefaultInPlaceState = InPlaceState.Expanded;

                // We default to lined input. We also default to opening TIP upward and expanded.
                tipSignature.DefaultInputArea = PanelInputArea.WritingPad;
                tipSignature.PreferredInPlaceDirection = InPlaceDirection.Top;
                tipSignature.DefaultInPlaceState = InPlaceState.Expanded;

                // Add Event Handlers for tipShared, so that we can track tipShared's events.
                tipShared.InputAreaChanging += new EventHandler<InputAreaChangeEventArgs>(tip_InputAreaChanging);
                tipShared.InputAreaChanged += new EventHandler<InputAreaChangeEventArgs>(tip_InputAreaChanged);

                tipShared.InPlaceVisibilityChanging += new EventHandler<InPlaceVisibilityChangeEventArgs>(tip_InPlaceVisibilityChanging);
                tipShared.InPlaceVisibilityChanged += new EventHandler<InPlaceVisibilityChangeEventArgs>(tip_InPlaceVisibilityChanged);
                
                tipShared.CorrectionModeChanging += new EventHandler<CorrectionModeChangeEventArgs>(tip_CorrectionModeChanging);
                tipShared.CorrectionModeChanged += new EventHandler<CorrectionModeChangeEventArgs>(tip_CorrectionModeChanged);
                
                tipShared.InPlaceSizeChanging += new EventHandler<InPlaceSizeChangeEventArgs>(tip_InPlaceSizeChanging);
                tipShared.InPlaceSizeChanged += new EventHandler<InPlaceSizeChangeEventArgs>(tip_InPlaceSizeChanged);
                
                tipShared.InPlaceStateChanging += new EventHandler<InPlaceStateChangeEventArgs>(tip_InPlaceStateChanging);
                tipShared.InPlaceStateChanged += new EventHandler<InPlaceStateChangeEventArgs>(tip_InPlaceStateChanged);

                tipShared.TextInserting += new EventHandler<TextInsertionEventArgs>(tip_TextInserting);
                tipShared.TextInserted += new EventHandler<TextInsertionEventArgs>(tip_TextInserted);

                // Add Event Handler for tipSignature, so that we can draw ink input on panelForDrawing.
                tipSignature.TextInserting += new EventHandler<TextInsertionEventArgs>(tip_TextInserting);
                tipSignature.TextInserted += new EventHandler<TextInsertionEventArgs>(tip_TextInserted);
            }
            catch (System.Runtime.InteropServices.COMException ex)
            {
                // If the peninputpanel objects cannot be created, set them
                // to null. Other code that references these objects will need
                // to check 
                if (ex.ErrorCode == ObjectsNotInstalledError)
                {
                    tipShared = null;
                    tipPolicyNumber = null;
                    tipName = null;
                }
                else
                {
                    throw; // rethrow unexpected exception
                }
            }

            // Initialize the four different layers of ink on the vehicle diagram:  
            // vehihcle body, windows, tires, and headlights.
            inkLayers = new InkLayer[4];
            inkLayers[0] = new InkLayer(new Ink(), Color.Red, false);
            inkLayers[1] = new InkLayer(new Ink(), Color.Violet, false);
            inkLayers[2] = new InkLayer(new Ink(), Color.LightGreen, false);
            inkLayers[3] = new InkLayer(new Ink(), Color.Aqua, false);

            // By default, select the first ink layer
            lstAnnotationLayer.SelectedIndex = 0;
            inkPictVehicle.DefaultDrawingAttributes.Color = inkLayers[lstAnnotationLayer.SelectedIndex].ActiveColor;

        }

        /// <summary>
        ///  Event Handler from lstAnnotationLayer->SelectedIndexChanged Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void lstAnnotationLayer_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            // Provided that the new selected index value is different than
            // the previous value...
            if (lstAnnotationLayer.SelectedIndex != selectedIndex) 
            {
                if (!inkPictVehicle.CollectingInk)
                {
                    // Set the ink and visibility of the current ink layer
                    inkPictVehicle.DefaultDrawingAttributes.Color = inkLayers[lstAnnotationLayer.SelectedIndex].ActiveColor;
                    chHideLayer.Checked = inkLayers[lstAnnotationLayer.SelectedIndex].Hidden;

                    // Set the active ink object to the selected ink
                    // Note that if the current layer is not visible, empty
                    // ink is used to prevent flicker.
                    inkPictVehicle.InkEnabled = false;
                    inkPictVehicle.Ink = chHideLayer.Checked?emptyInk:inkLayers[lstAnnotationLayer.SelectedIndex].ActiveInk;
                    inkPictVehicle.InkEnabled = !chHideLayer.Checked;
            
                    selectedIndex = lstAnnotationLayer.SelectedIndex;

                    this.Refresh();
                }
                else 
                {
                    // If ink collection is enabled, the active ink cannot be changed
                    // and it is necessary to restore the selection to its previous value.
                    lstAnnotationLayer.SelectedIndex = selectedIndex;
                    MessageBox.Show("Cannot change active ink while collecting ink.");
                }
            }
        }

        /// <summary>
        /// Event Handler from chHideLayer->CheckChanged Event
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void chHideLayer_CheckedChanged(object sender, System.EventArgs e)
        {
            // Provided that the new checked hidden value is different than
            // the previous value...
            if (chHideLayer.Checked != selectedHidden) 
            {
                if (!inkPictVehicle.CollectingInk)
                {
                    // Update the array indicating the visibility of each ink layer
                    inkLayers[lstAnnotationLayer.SelectedIndex].Hidden = chHideLayer.Checked;

                    // Set the active ink object to the selected ink
                    // Note that if the current layer is not visible, empty
                    // ink is used to prevent flicker.
                    inkPictVehicle.InkEnabled = false;
                    inkPictVehicle.Ink = chHideLayer.Checked?emptyInk:inkLayers[lstAnnotationLayer.SelectedIndex].ActiveInk;
         
                    // If the layer is marked hidden, disable ink collections
                    inkPictVehicle.InkEnabled = !chHideLayer.Checked;

                    // Update the previous checkbox value to reflect the current
                    selectedHidden = chHideLayer.Checked;

                    this.Refresh();
                }
                else 
                {
                    // If ink collection is enabled, the active ink cannot be changed
                    // and it is necessary to restore the checkbox to its previous value.
                    chHideLayer.Checked = selectedHidden;
                    MessageBox.Show("Cannot change visibility while collecting ink.");
                }
            }
        }

        /// <summary>
        /// Event Handler from inkPictVehicle->Paint Event
        /// 
        /// It is necessary to draw four different types of ink
        /// and the ink picture only has one ink associated with it.
        /// As a result, the autoredraw feature is turned off 
        /// and the paint event is handled.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void inkPictVehicle_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
        {
            // Cycle through each ink layer.  If it is visible, paint it.
            // Note that it is necessary to customize the paint
            // behavior, since we want to hide/show different ink layers.
            foreach (InkLayer layer in inkLayers)
            {
                if (!layer.Hidden)
                {
                    inkPictVehicle.Renderer.Draw(e.Graphics,layer.ActiveInk.Strokes);
                }
            }          
        }

        /// <summary>
        /// Event Handler for inkEdPolicyNumber->Enter Event
        /// </summary>
        private void inkEdPolicyNumber_Enter(object sender, System.EventArgs e)
        {
            if (tipPolicyNumber != null)
            {
                tipPolicyNumber.SetInPlaceHoverTargetPosition(0,0);
            }
        }

        /// <summary>
        /// Event Handler for inkEdYear->Enter Event
        /// 
        /// As the edit control receives focus, we attach the shared PenInputPanel to it.
        /// The Year, Make, and Model fields demonstrate how to share a single PenInputPanel
        /// among multiple controls
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void inkEdYear_Enter(object sender, System.EventArgs e)
        {
            if (tipShared != null)
            {
                tipShared.AttachedEditControl = inkEdYear;

                // Set default input area to be Keyboard.
                tipShared.DefaultInputArea = PanelInputArea.Keyboard;

                // Set default inplace state to be expanded.
                tipShared.DefaultInPlaceState = InPlaceState.Expanded;

                // Set preferred In-Place directin to top. This way the TIP does not cover controls below.
                tipShared.PreferredInPlaceDirection = InPlaceDirection.Top;
            }
        }

        /// <summary>
        /// Event Handler for inkEdMake->Enter Event
        /// 
        /// As the edit control receives focus, we attach the shared PenInputPanel to it.
        /// The Year, Make, and Model fields demonstrate how to share a single PenInputPanel
        /// among multiple controls
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void inkEdMake_Enter(object sender, System.EventArgs e)
        {

            if (tipShared != null)
            {
                // Attach the shared TextInputPanel to the Make field
                tipShared.AttachedEditControl = inkEdMake;

                // Set default input area to be Writing Pad.
                tipShared.DefaultInputArea = PanelInputArea.WritingPad;

                // Set default inplace state to be expanded.
                tipShared.DefaultInPlaceState = InPlaceState.Expanded;

                // If there is enough room above, set the TIP to 100 pixels above the text box
                // Get the bounding rectangle of TIP
                Rectangle recTip = tipShared.InPlaceBoundingRectangle;
                // Get the height of pop up correction comb
                int iRecPopUp = tipShared.PopUpCorrectionHeight;
                
                //Get the Y coordinate of the Edit box
                int iEditY = inkEdMake.Location.Y + this.Top;

                if (iEditY > recTip.Height + iRecPopUp + 100) 
                {
                    //Reset TIP's location
                    tipShared.SetInPlacePosition(recTip.Left, iRecPopUp + 100, CorrectionPosition.Top);
                }
            }
        }

        /// <summary>
        /// Event Handler for inkEdModel->Enter Event
        /// 
        /// As the edit control receives focus, we attach the shared PenInputPanel to it.
        /// The Year, Make, and Model fields demonstrate how to share a single PenInputPanel
        /// among multiple controls
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void inkEdModel_Enter(object sender, System.EventArgs e)
        {
            if (tipShared != null)
            {

                // Attach the shared TextInputPanel to the Model fi
                tipShared.AttachedEditControl = inkEdModel;
                
                // Set default input area to be Writing Pad.
                tipShared.DefaultInputArea = PanelInputArea.WritingPad;

                // Set default inplace state to be expanded.
                tipShared.DefaultInPlaceState = InPlaceState.Expanded;

                // Set the PreferredInPlaceDirection to Bottom.
                tipShared.PreferredInPlaceDirection = InPlaceDirection.Bottom;
            }
        }

        /// <summary>
        /// Event Handlers for TextInputPanel Events.
        /// 
        /// When TIP UI changes, or ink is inserting/inserted, events are triggered respectively.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        void tip_TextInserted(object sender, TextInsertionEventArgs e)
        {
            seForm.m_richTextBox1.Text += "TextInserted: " + e.GetInk().Length.ToString() + " Ink objects" + Environment.NewLine;

            //If this event is triggered by tipSignature, store the ink input.
            if (sender.Equals(tipSignature))
            {

                // Remove the old ink
                this.insertedInk.Clear();

                //Store ink collected from the TIP
                foreach(Ink ink in e.GetInk())
                {
                    insertedInk.Add(ink);
                }

                //Refresh drawing panel
                panelForSignature.Refresh();
            }
        }

        void tip_TextInserting(object sender, TextInsertionEventArgs e)
        {
            seForm.m_richTextBox1.Text += "TextInserting: " + e.GetInk().Length.ToString() + " Ink objects" + Environment.NewLine;
        }

        void tip_InPlaceStateChanging(object sender, InPlaceStateChangeEventArgs e)
        {
            seForm.m_richTextBox1.Text += "InPlaceStateChanging" + Environment.NewLine;
        }

        void tip_InPlaceStateChanged(object sender, InPlaceStateChangeEventArgs e)
        {
            seForm.m_richTextBox1.Text += "InPlaceStateChanged" + Environment.NewLine;
            seForm.m_richTextBox1.Text += "CurrentInPlaceState is " + tipShared.CurrentInPlaceState + Environment.NewLine;
        }

        void tip_InPlaceSizeChanging(object sender, InPlaceSizeChangeEventArgs e)
        {
            seForm.m_richTextBox1.Text += "InPlaceSizeChanging" + Environment.NewLine;
        }

        void tip_InPlaceSizeChanged(object sender, InPlaceSizeChangeEventArgs e)
        {
            seForm.m_richTextBox1.Text += "InPlaceSizeChanged" + Environment.NewLine;
        }

        void tip_CorrectionModeChanging(object sender, CorrectionModeChangeEventArgs e)
        {
            seForm.m_richTextBox1.Text += "CorrectionModeChanging" + Environment.NewLine;
        }

        void tip_CorrectionModeChanged(object sender, CorrectionModeChangeEventArgs e)
        {
            seForm.m_richTextBox1.Text += "CorrectionModeChanged" + Environment.NewLine;
            seForm.m_richTextBox1.Text += "CurrentCorrectionMode is " + tipShared.CurrentCorrectionMode + Environment.NewLine;
        }

        void tip_InPlaceVisibilityChanging(object sender, InPlaceVisibilityChangeEventArgs e)
        {
            seForm.m_richTextBox1.Text += "InPlaceVisibilityChanging" + Environment.NewLine;
        }

        void tip_InPlaceVisibilityChanged(object sender, InPlaceVisibilityChangeEventArgs e)
        {
            seForm.m_richTextBox1.Text += "InPlaceVisibilityChanged" + Environment.NewLine;
            // Set the status text when the Input Panel opens or closes
            if (e.Visible)
            {
                seForm.m_richTextBox1.Text += "TextInputPanel is visible" + Environment.NewLine;
            }
            else
            {
                seForm.m_richTextBox1.Text += "TextInputPanel is not visible" + Environment.NewLine;
            }
        }

        void tip_InputAreaChanging(object sender, InputAreaChangeEventArgs e)
        {
            // Change the status to show the new input area.
            seForm.m_richTextBox1.Text += "InputAreaChanging from " + e.OldArea.ToString() + " to " + e.NewArea.ToString() + Environment.NewLine;
        }

        void tip_InputAreaChanged(object sender, InputAreaChangeEventArgs e)
        {
            // Change the status to show the new input area.
            seForm.m_richTextBox1.Text += "CurrentInputArea is " + tipShared.CurrentInputArea + Environment.NewLine;
        }

        /// <summary>
        /// Show Event Log
        /// </summary>
        private void showEventButton_Click(object sender, EventArgs e)
        {
            seForm.ShowDialog();
        }

        /// <summary>
        /// When the panelForSignature refreshes, it draws the ink that was retrieved from
        /// the TIP.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void panelForSignature_Paint(object sender, PaintEventArgs e)
        {
            Point thePoint = Point.Empty;
            thePoint = new Point(panelForSignature.Width, panelForSignature.Height);
            theInkOverlay.Renderer.PixelToInkSpace(e.Graphics, ref thePoint);

            Rectangle theRectangle = new Rectangle(0, 0, thePoint.X, thePoint.Y);

            theInkOverlay.Ink.DeleteStrokes();
            
            foreach (Ink ink in insertedInk)
            {
                if (null != ink)
                {
                    theInkOverlay.Ink.AddStrokesAtRectangle(ink.Strokes, theRectangle);
                }
            }
        }
    }
}