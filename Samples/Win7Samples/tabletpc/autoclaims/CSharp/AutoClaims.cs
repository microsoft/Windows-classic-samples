// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  File: AutoClaims.cs
//  AutoClaims Sample Application
//
//  This sample program simulates an auto insurance claim
//  form. It uses the InkEdit control to allow the user to 
//  enter vehicle information and the InkPicture control to
//  illustrate car damage.  The car damage diagram allows
//  the user to select from multiple ink layers;  each layer
//  has its own color and visibility setting.  
//
//  The features used are: InkEdit, InkPicture, providing
//  multiple layers of ink within a control.
//
//--------------------------------------------------------------------------

using System;
using System.Drawing;
using System.Windows.Forms;
using System.Data;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;

namespace AutoClaims
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

        // Declare the array of ink layers used the vehicle illustration.
        InkLayer[] inkLayers;

        // Declare an empty ink object (used when we don't want to draw
        // any ink).
        Ink emptyInk;

        // Declare a value to hold the index of selected ink
        int selectedIndex = -1;

        // Declare a value to hold whether the selected ink is hidden
        bool selectedHidden = false;

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
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(AutoClaims));
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
            this.SuspendLayout();
            // 
            // lblVehicleDamage
            // 
            this.lblVehicleDamage.AutoSize = true;
            this.lblVehicleDamage.BackColor = System.Drawing.Color.White;
            this.lblVehicleDamage.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblVehicleDamage.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblVehicleDamage.Location = new System.Drawing.Point(8, 408);
            this.lblVehicleDamage.Name = "lblVehicleDamage";
            this.lblVehicleDamage.Size = new System.Drawing.Size(256, 20);
            this.lblVehicleDamage.TabIndex = 58;
            this.lblVehicleDamage.Text = "Description of vehicle damage";
            this.lblVehicleDamage.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblVehicleDescription
            // 
            this.lblVehicleDescription.AutoSize = true;
            this.lblVehicleDescription.BackColor = System.Drawing.Color.White;
            this.lblVehicleDescription.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblVehicleDescription.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblVehicleDescription.Location = new System.Drawing.Point(8, 240);
            this.lblVehicleDescription.Name = "lblVehicleDescription";
            this.lblVehicleDescription.Size = new System.Drawing.Size(386, 20);
            this.lblVehicleDescription.TabIndex = 56;
            this.lblVehicleDescription.Text = "Enter the involved vehicle listed on the policy ";
            this.lblVehicleDescription.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblModel
            // 
            this.lblModel.AutoSize = true;
            this.lblModel.BackColor = System.Drawing.Color.Gainsboro;
            this.lblModel.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblModel.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblModel.Location = new System.Drawing.Point(112, 360);
            this.lblModel.Name = "lblModel";
            this.lblModel.Size = new System.Drawing.Size(73, 23);
            this.lblModel.TabIndex = 52;
            this.lblModel.Text = "Model:";
            this.lblModel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblYear
            // 
            this.lblYear.AutoSize = true;
            this.lblYear.BackColor = System.Drawing.Color.Gainsboro;
            this.lblYear.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblYear.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblYear.Location = new System.Drawing.Point(48, 280);
            this.lblYear.Name = "lblYear";
            this.lblYear.Size = new System.Drawing.Size(135, 23);
            this.lblYear.TabIndex = 51;
            this.lblYear.Text = "Year (YYYY):";
            this.lblYear.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblMake
            // 
            this.lblMake.AutoSize = true;
            this.lblMake.BackColor = System.Drawing.Color.Gainsboro;
            this.lblMake.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblMake.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblMake.Location = new System.Drawing.Point(112, 320);
            this.lblMake.Name = "lblMake";
            this.lblMake.Size = new System.Drawing.Size(67, 23);
            this.lblMake.TabIndex = 50;
            this.lblMake.Text = "Make:";
            this.lblMake.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblPolicyNumber
            // 
            this.lblPolicyNumber.AutoSize = true;
            this.lblPolicyNumber.BackColor = System.Drawing.Color.Gainsboro;
            this.lblPolicyNumber.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblPolicyNumber.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblPolicyNumber.Location = new System.Drawing.Point(32, 136);
            this.lblPolicyNumber.Name = "lblPolicyNumber";
            this.lblPolicyNumber.Size = new System.Drawing.Size(157, 23);
            this.lblPolicyNumber.TabIndex = 47;
            this.lblPolicyNumber.Text = "Policy Number:";
            this.lblPolicyNumber.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblInsuredName
            // 
            this.lblInsuredName.AutoSize = true;
            this.lblInsuredName.BackColor = System.Drawing.Color.Gainsboro;
            this.lblInsuredName.Font = new System.Drawing.Font("Tahoma", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblInsuredName.ImageAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.lblInsuredName.Location = new System.Drawing.Point(32, 176);
            this.lblInsuredName.Name = "lblInsuredName";
            this.lblInsuredName.Size = new System.Drawing.Size(154, 23);
            this.lblInsuredName.TabIndex = 46;
            this.lblInsuredName.Text = "Insured Name:";
            this.lblInsuredName.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // chHideLayer
            // 
            this.chHideLayer.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.chHideLayer.Location = new System.Drawing.Point(416, 576);
            this.chHideLayer.Name = "chHideLayer";
            this.chHideLayer.Size = new System.Drawing.Size(144, 24);
            this.chHideLayer.TabIndex = 61;
            this.chHideLayer.Text = "Hide Layer";
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
            this.pictVehicleBox.Size = new System.Drawing.Size(184, 120);
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
            this.pictPolicyBox.Size = new System.Drawing.Size(184, 80);
            this.pictPolicyBox.TabIndex = 44;
            this.pictPolicyBox.TabStop = false;
            // 
            // pictHorzontalBar1
            // 
            this.pictHorzontalBar1.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.pictHorzontalBar1.Location = new System.Drawing.Point(8, 112);
            this.pictHorzontalBar1.Name = "pictHorzontalBar1";
            this.pictHorzontalBar1.Size = new System.Drawing.Size(536, 2);
            this.pictHorzontalBar1.TabIndex = 43;
            this.pictHorzontalBar1.TabStop = false;
            // 
            // lblInstructions
            // 
            this.lblInstructions.Font = new System.Drawing.Font("Tahoma", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblInstructions.Location = new System.Drawing.Point(96, 48);
            this.lblInstructions.Name = "lblInstructions";
            this.lblInstructions.Size = new System.Drawing.Size(456, 56);
            this.lblInstructions.TabIndex = 42;
            this.lblInstructions.Text = "Enter the required information for the policy applicable to the auto claim you ar" +
                "e submitting. Enter the vehicle exactly as it appears on the policy or insurance" +
                " card.";
            // 
            // lblTitle
            // 
            this.lblTitle.Font = new System.Drawing.Font("Tahoma", 21.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lblTitle.Location = new System.Drawing.Point(8, 8);
            this.lblTitle.Name = "lblTitle";
            this.lblTitle.Size = new System.Drawing.Size(288, 32);
            this.lblTitle.TabIndex = 41;
            this.lblTitle.Text = "Auto Claim";
            // 
            // pictCheckmarks
            // 
            this.pictCheckmarks.BackColor = System.Drawing.Color.White;
            this.pictCheckmarks.BackgroundImage = ((System.Drawing.Bitmap)(resources.GetObject("pictCheckmarks.BackgroundImage")));
            this.pictCheckmarks.Location = new System.Drawing.Point(32, 40);
            this.pictCheckmarks.Name = "pictCheckmarks";
            this.pictCheckmarks.Size = new System.Drawing.Size(40, 72);
            this.pictCheckmarks.TabIndex = 66;
            this.pictCheckmarks.TabStop = false;
            // 
            // inkPictVehicle
            // 
            this.inkPictVehicle.AutoRedraw = false;
            this.inkPictVehicle.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.inkPictVehicle.Image = ((System.Drawing.Bitmap)(resources.GetObject("inkPictVehicle.Image")));
            this.inkPictVehicle.Location = new System.Drawing.Point(32, 440);
            this.inkPictVehicle.MarginX = -1;
            this.inkPictVehicle.MarginY = -1;
            this.inkPictVehicle.Name = "inkPictVehicle";
            this.inkPictVehicle.Size = new System.Drawing.Size(352, 232);
            this.inkPictVehicle.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.inkPictVehicle.TabIndex = 67;
            this.inkPictVehicle.Paint += new System.Windows.Forms.PaintEventHandler(this.inkPictVehicle_Paint);
            // 
            // lstAnnotationLayer
            // 
            this.lstAnnotationLayer.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.lstAnnotationLayer.Font = new System.Drawing.Font("Tahoma", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.lstAnnotationLayer.ItemHeight = 19;
            this.lstAnnotationLayer.Items.AddRange(new object[] {
                                                                    "Body",
                                                                    "Windows",
                                                                    "Tires",
                                                                    "Headlights"});
            this.lstAnnotationLayer.Location = new System.Drawing.Point(416, 432);
            this.lstAnnotationLayer.Name = "lstAnnotationLayer";
            this.lstAnnotationLayer.Size = new System.Drawing.Size(104, 135);
            this.lstAnnotationLayer.TabIndex = 68;
            this.lstAnnotationLayer.SelectedIndexChanged += new System.EventHandler(this.lstAnnotationLayer_SelectedIndexChanged);
            // 
            // inkEdPolicyNumber
            // 
            this.inkEdPolicyNumber.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdPolicyNumber.Location = new System.Drawing.Point(208, 136);
            this.inkEdPolicyNumber.Name = "inkEdPolicyNumber";
            this.inkEdPolicyNumber.Size = new System.Drawing.Size(312, 32);
            this.inkEdPolicyNumber.TabIndex = 69;
            this.inkEdPolicyNumber.Text = "";
            this.inkEdPolicyNumber.UseMouseForInput = true;
            // 
            // inkEdName
            // 
            this.inkEdName.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdName.Location = new System.Drawing.Point(208, 176);
            this.inkEdName.Name = "inkEdName";
            this.inkEdName.Size = new System.Drawing.Size(312, 32);
            this.inkEdName.TabIndex = 70;
            this.inkEdName.Text = "";
            this.inkEdName.UseMouseForInput = true;
            // 
            // inkEdYear
            // 
            this.inkEdYear.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdYear.Location = new System.Drawing.Point(208, 280);
            this.inkEdYear.Name = "inkEdYear";
            this.inkEdYear.Size = new System.Drawing.Size(312, 32);
            this.inkEdYear.TabIndex = 71;
            this.inkEdYear.Text = "";
            this.inkEdYear.UseMouseForInput = true;
            // 
            // inkEdMake
            // 
            this.inkEdMake.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdMake.Location = new System.Drawing.Point(208, 320);
            this.inkEdMake.Name = "inkEdMake";
            this.inkEdMake.Size = new System.Drawing.Size(312, 32);
            this.inkEdMake.TabIndex = 72;
            this.inkEdMake.Text = "";
            this.inkEdMake.UseMouseForInput = true;
            // 
            // inkEdModel
            // 
            this.inkEdModel.Cursor = System.Windows.Forms.Cursors.Default;
            this.inkEdModel.Location = new System.Drawing.Point(208, 360);
            this.inkEdModel.Name = "inkEdModel";
            this.inkEdModel.Size = new System.Drawing.Size(312, 32);
            this.inkEdModel.TabIndex = 73;
            this.inkEdModel.Text = "";
            this.inkEdModel.UseMouseForInput = true;
            // 
            // AutoClaims
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.AutoScroll = true;
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(560, 694);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.inkEdModel,
                                                                          this.inkEdMake,
                                                                          this.inkEdYear,
                                                                          this.inkEdName,
                                                                          this.inkEdPolicyNumber,
                                                                          this.lstAnnotationLayer,
                                                                          this.inkPictVehicle,
                                                                          this.lblVehicleDamage,
                                                                          this.lblVehicleDescription,
                                                                          this.lblModel,
                                                                          this.lblYear,
                                                                          this.lblMake,
                                                                          this.lblPolicyNumber,
                                                                          this.lblInsuredName,
                                                                          this.chHideLayer,
                                                                          this.pictHorzontalBar2,
                                                                          this.pictVehicleBox,
                                                                          this.pictHorzontalBar3,
                                                                          this.pictPolicyBox,
                                                                          this.pictHorzontalBar1,
                                                                          this.lblInstructions,
                                                                          this.lblTitle,
                                                                          this.pictCheckmarks});
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "AutoClaims";
            this.Text = "AutoClaims";
            this.Load += new System.EventHandler(this.AutoClaims_Load);
            this.ResumeLayout(false);

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
        /// configures the insertion mode and the ink used by
        /// this application.
        /// </summary>
        /// <param name="sender">The control that raised the event.</param>
        /// <param name="e">The event arguments.</param>
        private void AutoClaims_Load(object sender, System.EventArgs e)
        {
            // Initialize the empty ink
            emptyInk = new Ink();

            // Initialize the four different layers of ink on the vehicle diagram:  
            // vehicle body, windows, tires, and headlights.
            inkLayers = new InkLayer[4];
            inkLayers[0] = new InkLayer(new Ink(), Color.Red, false);
            inkLayers[1] = new InkLayer(new Ink(), Color.Violet, false);
            inkLayers[2] = new InkLayer(new Ink(), Color.LightGreen, false);
            inkLayers[3] = new InkLayer(new Ink(), Color.Aqua, false);

            // By default, select the first ink layer
            lstAnnotationLayer.SelectedIndex = 0;
            inkPictVehicle.DefaultDrawingAttributes.Color = inkLayers[lstAnnotationLayer.SelectedIndex].ActiveColor;

            // Currently there are recognizers that do not support regular expression 
            // input scopes, thus raising exceptions.  This try catch handler copes 
            // with these differences in the recognizer. For policy numbers, nothing 
            // is being done in reaction to this exception though since there is no 
            // factoid that serves the same purpose.
            try 
            {
                inkEdPolicyNumber.Factoid = "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)-(0|1|2|3|4|5|6|7|" +
                    "8|9)(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)";
            } 
            catch
            {
                // Do nothing
            }
            
            // There are also recognizers that do not support common input scopes as
            // well.  This try catch handler copes with these differences in the 
            // recognizer. For year, the NUMBER factoid is being set in reaction to 
            // this exception.
            try
            {
                inkEdYear.Factoid = "(!IS_DATE_YEAR)";
            }
            catch
            {
                inkEdYear.Factoid = Factoid.Number;
            }
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
                    // Set the ink and visiblity of the current ink layer
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
                    MessageBox.Show("Cannot change visiblity while collecting ink.");
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

    }
}
