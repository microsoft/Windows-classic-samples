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

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Messaging;
using Microsoft.Win32;

//
// Line related struct and functions
//
namespace Util
{
	public struct line
	{
		public int XFrom;
		public int YFrom;
		public int XTo;
		public int YTo;
	}

	public class HelperFunctions
	{
		public string LineToString(int X1, int Y1, int X2, int Y2)
		{
			string strX1;
			string strY1;
			string strX2;
			string strY2;
			string strResult;
			strX1 = X1.ToString().PadLeft(7,'0');
			strY1 = Y1.ToString().PadLeft(7,'0');
			strX2 = X2.ToString().PadLeft(7,'0');
			strY2 = Y2.ToString().PadLeft(7,'0');
			strResult=strX1+strY1+strX2+strY2;
			return strResult;
		}
		
		public void StringToLine(string strLine, ref line lineToDraw)
		{
			lineToDraw.XFrom = Convert.ToInt32(strLine.Substring(0,7));
			lineToDraw.YFrom = Convert.ToInt32(strLine.Substring(7,7));
			lineToDraw.XTo = Convert.ToInt32(strLine.Substring(14,7));
			lineToDraw.YTo = Convert.ToInt32(strLine.Substring(21,7));
		}
	}
}

namespace CSharp_Draw
{
	/// <summary>
	/// Form1 is the main form for sending and receiving Drawing commands using MSMQ
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		/// 

		//
		// The X and Y of the previous Line.
		//
		private int iLastX;
		private int iLastY;

		

		//
		// Specify a guid that will be used for the Category property of the queue created
		//
		const string guidDraw = "{151ceac0-acb5-11cf-8b51-0020af929546}";
		public string strName;

		//
		// Hold the information on the lines that were drawn in the picturebox. This
		// is used for refresh purposes.
		//
	
		private ArrayList lines = new ArrayList();
		

		//
		// Graphics Stuff
		//

		public System.Drawing.Graphics picture1Graphics;
		public System.Drawing.Pen BlackPen;
		public System.Drawing.Color Color;
		public System.Drawing.Font fontText;
		public System.Drawing.Brush brushText;
		public System.Drawing.FontFamily fontfamilyText;

		//
		// Message Queue Stuff
		//

		private System.Messaging.MessageQueue qFriend;  // The remote drawing queue
		private System.Messaging.MessageQueue qMe;		// My drawing queue     
		private bool bDSEnabled;						// Are we working in a Domain or Workgroup mode.
		private bool bDirectMode;						// Connection using Direct Mode


		Util.HelperFunctions HP = new Util.HelperFunctions();
		private System.Windows.Forms.Button btnAttach;
		private System.Windows.Forms.TextBox textFriendName;
		private System.Windows.Forms.GroupBox groupSendType;
		private System.Windows.Forms.RadioButton radioSendRec;
		private System.Windows.Forms.RadioButton radioSendExp;
		private System.Windows.Forms.TextBox textFriendComputer;
		private System.Windows.Forms.Label lblRemoteQueue;
		private System.Windows.Forms.Label lblRemoteComputer;
		private System.Windows.Forms.Label lblDSConnection;
		private System.Windows.Forms.Button btnConnect;
		private System.Windows.Forms.Button btnCancel;
		private System.Windows.Forms.GroupBox groupConnectionMode;
		private System.Windows.Forms.RadioButton radioDSDisabled;
		private System.Windows.Forms.RadioButton radioDSEnabled;
		private System.Windows.Forms.TextBox textChars;
		private System.Windows.Forms.PictureBox picture1;

		private System.ComponentModel.Container components;

		public Form1()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// Prepare the Graphics elements for the drawing
			//
			
			picture1Graphics = picture1.CreateGraphics();
			Color = new System.Drawing.Color();
			BlackPen = new System.Drawing.Pen(Color.Black,1);
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.radioDSDisabled = new System.Windows.Forms.RadioButton();
			this.textChars = new System.Windows.Forms.TextBox();
			this.btnAttach = new System.Windows.Forms.Button();
			this.radioSendExp = new System.Windows.Forms.RadioButton();
			this.lblDSConnection = new System.Windows.Forms.Label();
			this.textFriendName = new System.Windows.Forms.TextBox();
			this.groupConnectionMode = new System.Windows.Forms.GroupBox();
			this.radioDSEnabled = new System.Windows.Forms.RadioButton();
			this.textFriendComputer = new System.Windows.Forms.TextBox();
			this.lblRemoteQueue = new System.Windows.Forms.Label();
			this.groupSendType = new System.Windows.Forms.GroupBox();
			this.radioSendRec = new System.Windows.Forms.RadioButton();
			this.btnCancel = new System.Windows.Forms.Button();
			this.picture1 = new System.Windows.Forms.PictureBox();
			this.btnConnect = new System.Windows.Forms.Button();
			this.lblRemoteComputer = new System.Windows.Forms.Label();
			this.groupConnectionMode.SuspendLayout();
			this.groupSendType.SuspendLayout();
			this.SuspendLayout();
			// 
			// radioDSDisabled
			// 
			this.radioDSDisabled.Location = new System.Drawing.Point(8, 56);
			this.radioDSDisabled.Name = "radioDSDisabled";
			this.radioDSDisabled.Size = new System.Drawing.Size(216, 16);
			this.radioDSDisabled.TabIndex = 1;
			this.radioDSDisabled.Text = "Private queue";
			// 
			// textChars
			// 
			this.textChars.Location = new System.Drawing.Point(16, 272);
			this.textChars.Name = "textChars";
			this.textChars.Size = new System.Drawing.Size(384, 20);
			this.textChars.TabIndex = 11;
			this.textChars.Text = "";
			this.textChars.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textChars_KeyPress);
			// 
			// btnAttach
			// 
			this.btnAttach.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F);
			this.btnAttach.Location = new System.Drawing.Point(312, 304);
			this.btnAttach.Name = "btnAttach";
			this.btnAttach.Size = new System.Drawing.Size(88, 24);
			this.btnAttach.TabIndex = 3;
			this.btnAttach.Text = "Start Sending";
			this.btnAttach.Click += new System.EventHandler(this.btnAttach_Click);
			// 
			// radioSendExp
			// 
			this.radioSendExp.Checked = true;
			this.radioSendExp.Location = new System.Drawing.Point(16, 24);
			this.radioSendExp.Name = "radioSendExp";
			this.radioSendExp.Size = new System.Drawing.Size(104, 16);
			this.radioSendExp.TabIndex = 0;
			this.radioSendExp.TabStop = true;
			this.radioSendExp.Text = "&Express";
			// 
			// lblDSConnection
			// 
			this.lblDSConnection.Location = new System.Drawing.Point(16, 384);
			this.lblDSConnection.Name = "lblDSConnection";
			this.lblDSConnection.Size = new System.Drawing.Size(100, 16);
			this.lblDSConnection.TabIndex = 7;
			// 
			// textFriendName
			// 
			this.textFriendName.Location = new System.Drawing.Point(160, 304);
			this.textFriendName.Name = "textFriendName";
			this.textFriendName.Size = new System.Drawing.Size(144, 20);
			this.textFriendName.TabIndex = 2;
			this.textFriendName.Text = "";
			this.textFriendName.TextChanged += new System.EventHandler(this.textFriendName_TextChanged);
			// 
			// groupConnectionMode
			// 
			this.groupConnectionMode.Controls.AddRange(new System.Windows.Forms.Control[] {
																							  this.radioDSDisabled,
																							  this.radioDSEnabled});
			this.groupConnectionMode.Location = new System.Drawing.Point(96, 304);
			this.groupConnectionMode.Name = "groupConnectionMode";
			this.groupConnectionMode.Size = new System.Drawing.Size(232, 96);
			this.groupConnectionMode.TabIndex = 10;
			this.groupConnectionMode.TabStop = false;
			this.groupConnectionMode.Text = "Queue Type";
			// 
			// radioDSEnabled
			// 
			this.radioDSEnabled.Checked = true;
			this.radioDSEnabled.Location = new System.Drawing.Point(8, 24);
			this.radioDSEnabled.Name = "radioDSEnabled";
			this.radioDSEnabled.Size = new System.Drawing.Size(208, 16);
			this.radioDSEnabled.TabIndex = 0;
			this.radioDSEnabled.TabStop = true;
			this.radioDSEnabled.Text = "Public queue";
			// 
			// textFriendComputer
			// 
			this.textFriendComputer.Location = new System.Drawing.Point(160, 336);
			this.textFriendComputer.Name = "textFriendComputer";
			this.textFriendComputer.Size = new System.Drawing.Size(144, 20);
			this.textFriendComputer.TabIndex = 6;
			this.textFriendComputer.Text = "";
			this.textFriendComputer.TextChanged += new System.EventHandler(this.textFriendComputer_TextChanged);
			// 
			// lblRemoteQueue
			// 
			this.lblRemoteQueue.Location = new System.Drawing.Point(16, 304);
			this.lblRemoteQueue.Name = "lblRemoteQueue";
			this.lblRemoteQueue.Size = new System.Drawing.Size(128, 16);
			this.lblRemoteQueue.TabIndex = 1;
			this.lblRemoteQueue.Text = "Send drawing to queue:";
			// 
			// groupSendType
			// 
			this.groupSendType.Controls.AddRange(new System.Windows.Forms.Control[] {
																						this.radioSendRec,
																						this.radioSendExp});
			this.groupSendType.Location = new System.Drawing.Point(256, 368);
			this.groupSendType.Name = "groupSendType";
			this.groupSendType.Size = new System.Drawing.Size(136, 72);
			this.groupSendType.TabIndex = 4;
			this.groupSendType.TabStop = false;
			this.groupSendType.Text = "Type of delivery";
			// 
			// radioSendRec
			// 
			this.radioSendRec.Location = new System.Drawing.Point(16, 48);
			this.radioSendRec.Name = "radioSendRec";
			this.radioSendRec.Size = new System.Drawing.Size(104, 16);
			this.radioSendRec.TabIndex = 0;
			this.radioSendRec.Text = "Recoverable";
			// 
			// btnCancel
			// 
			this.btnCancel.Location = new System.Drawing.Point(224, 408);
			this.btnCancel.Name = "btnCancel";
			this.btnCancel.Size = new System.Drawing.Size(104, 32);
			this.btnCancel.TabIndex = 9;
			this.btnCancel.Text = "Cancel";
			this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
			// 
			// picture1
			// 
			this.picture1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
			this.picture1.Location = new System.Drawing.Point(16, 8);
			this.picture1.Name = "picture1";
			this.picture1.Size = new System.Drawing.Size(383, 257);
			this.picture1.TabIndex = 0;
			this.picture1.TabStop = false;
			this.picture1.Paint += new System.Windows.Forms.PaintEventHandler(this.picture1_Paint);
			this.picture1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.picture1_MouseUp);
			this.picture1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.picture1_MouseMove);
			this.picture1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.picture1_MouseDown);
			// 
			// btnConnect
			// 
			this.btnConnect.Location = new System.Drawing.Point(96, 408);
			this.btnConnect.Name = "btnConnect";
			this.btnConnect.Size = new System.Drawing.Size(104, 32);
			this.btnConnect.TabIndex = 8;
			this.btnConnect.Text = "OK";
			this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
			// 
			// lblRemoteComputer
			// 
			this.lblRemoteComputer.Location = new System.Drawing.Point(16, 336);
			this.lblRemoteComputer.Name = "lblRemoteComputer";
			this.lblRemoteComputer.Size = new System.Drawing.Size(144, 24);
			this.lblRemoteComputer.TabIndex = 5;
			this.lblRemoteComputer.Text = "Send drawing to computer:";
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(413, 450);
			this.Controls.AddRange(new System.Windows.Forms.Control[] {
																		  this.groupConnectionMode,
																		  this.textChars,
																		  this.btnCancel,
																		  this.btnConnect,
																		  this.lblDSConnection,
																		  this.textFriendComputer,
																		  this.groupSendType,
																		  this.btnAttach,
																		  this.textFriendName,
																		  this.lblRemoteQueue,
																		  this.picture1,
																		  this.lblRemoteComputer});
			this.Name = "Form1";
			this.Text = "Form1";
			this.groupConnectionMode.ResumeLayout(false);
			this.groupSendType.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			string strResult;

			//
			// Get the name of the local drawing queue
			//
			InputBox dlg = new InputBox();
			dlg.ShowDialog();
			//
			// strResult holds the name of my drawing queue
			//
			strResult = dlg.textBoxName.Text.ToString().ToUpper();
			if ((!dlg.bResult) || (strResult == "")) return;
			
			Form1 F;
			F = new Form1();
			F.strName = strResult;
			F.Text = "Listening to: " + strResult;

			F.textChars.Enabled = false;
			F.picture1.Enabled = false;
			
			//
			// Check if we are running on a DS Enabled machine. This can be found in the Registry.
			//
			Microsoft.Win32.RegistryKey hKey;
			Int32 iWG;
			try
			{
				hKey = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\Microsoft\MSMQ\Parameters");
				iWG = (System.Int32)hKey.GetValue(@"Workgroup");
				Registry.LocalMachine.Close();
				if (iWG == 0)
				{
					F.bDSEnabled = true;
				}
				else
				{
					F.bDSEnabled = false;
				}
			}
			catch(Exception e)
			{
				F.bDSEnabled = true;
			}
			
			if (F.bDSEnabled)
			{
				F.btnAttach.Visible = false;
				F.groupSendType.Visible = false;
				F.lblRemoteComputer.Visible=false;
				F.lblRemoteQueue.Visible=false;
				F.textFriendComputer.Visible=false;
				F.textFriendName.Visible=false;
			}
			else
			{
				F.bDirectMode = true;
				F.lblDSConnection.Text = "MODE: Workgroup";
				F.groupConnectionMode.Visible = false;
				F.btnCancel.Visible = false;
				F.btnConnect.Visible = false;

				try 
				{
					F.qMe=MessageQueue.Create(@".\private$\" + F.strName);
					F.qMe.Label = F.strName;
					F.qMe.Category = new System.Guid(guidDraw);
				}
				catch(MessageQueueException MQE)
				{
					F.qMe = new MessageQueue();
					F.qMe.Path = @".\private$\" + strResult;
				}
				//
				// Start Async - Receiving
				//
				F.qMe.Formatter = new ActiveXMessageFormatter();
				F.qMe.ReceiveCompleted  += new ReceiveCompletedEventHandler(F.qMe_ReceiveCompleted);
				F.qMe.BeginReceive();
			}

			Application.Run(F);
		}

		//
		// Picture Box mouse handlers events
		//
		private void picture1_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if ((e.Button!=MouseButtons.Left) || (e.X <= 0) || (e.Y <= 0)) return;
			
			iLastX = e.X;
			iLastY = e.Y;
		}

		private void picture1_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if ((e.Button!=MouseButtons.Left) || (e.X <= 0) || (e.Y <= 0)) return;
			
			Util.line lineToDraw = new Util.line();
			lineToDraw.XFrom=iLastX;
			lineToDraw.YFrom=iLastY;
			lineToDraw.XTo=e.X;
			lineToDraw.YTo=e.Y;
			AddLine(lineToDraw,picture1Graphics);
			iLastX=e.X;
			iLastY=e.Y;
			
			if (qFriend == null) return;
				
			System.Messaging.Message SendMsg;
			SendMsg = new System.Messaging.Message();
			SendMsg.Formatter = new ActiveXMessageFormatter();
			SendMsg.Priority=MessagePriority.Normal;
			SendMsg.Body=HP.LineToString(lineToDraw.XFrom,
										 lineToDraw.YFrom,
										 lineToDraw.XTo,
										 lineToDraw.YTo);
			SendMsg.Label=lineToDraw.XFrom.ToString()+","+
				          lineToDraw.YFrom.ToString()+" To "+
				          lineToDraw.XTo.ToString()+","+
				          lineToDraw.YTo.ToString();
			SendMsg.Recoverable = radioSendRec.Checked;
			qFriend.Send(SendMsg);
				
		}
		//
		// Clear the display
		//
		private void picture1_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button!=MouseButtons.Right) return;

			picture1Graphics.Clear(picture1.BackColor);
			lines.Clear();
			textChars.Text = "";
		}

		//
		// Lines managments functions
		//

		private void DrawLine(Util.line lineToDraw,System.Drawing.Graphics PG)
		{
			PG.DrawLine(BlackPen,
						lineToDraw.XFrom,
						lineToDraw.YFrom,
						lineToDraw.XTo,
						lineToDraw.YTo);
		}

		private void AddLine(Util.line lineToDraw,System.Drawing.Graphics PG)
		{
			DrawLine(lineToDraw,PG);
			lines.Add(lineToDraw);
		}

		// 
		// Characters drawing 
		//

		private void textChars_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
		{
            if (qFriend == null) return;
			
			System.Messaging.Message SendMsg;
			SendMsg = new System.Messaging.Message();
			SendMsg.Formatter = new ActiveXMessageFormatter();
			SendMsg.Priority=MessagePriority.AboveNormal;
			SendMsg.Body=e.KeyChar.ToString();
			SendMsg.Label="Key: "+e.KeyChar.ToString();
			SendMsg.Recoverable = radioSendRec.Checked;
			qFriend.Send(SendMsg);
		}

		private void DrawChar(string strCharToDraw)
		{
			if (strCharToDraw == "\b")
			{ // backspace
				if (textChars.Text.Length>0)
				{
					textChars.Text = textChars.Text.Substring(0,textChars.Text.Length -1);
				}
			}
			else
			{
				textChars.Text=textChars.Text+strCharToDraw;
			}
			return;	
		}

		//
		// Start sending messages to a friend
		//

		private void btnAttach_Click(object sender, System.EventArgs e)
		{
			MessageQueue[] mqlist;
			MessageQueueCriteria mqCriteria;

			if (textFriendName.Text == "")
			{
				MessageBox.Show("Please fill in queue name.","Missing value");
				return;
			}

			textFriendName.Text = textFriendName.Text.ToUpper();

			if (!bDirectMode) 
			{
				// 
				// Standard Connection mode = Working with public queues
				//

				mqCriteria = new MessageQueueCriteria();
				mqCriteria.Label=textFriendName.Text;
				mqCriteria.Category = new System.Guid(guidDraw);
				try
				{
					mqlist = MessageQueue.GetPublicQueues(mqCriteria);
				}
				catch(MessageQueueException MQE)
				{	
					MessageBox.Show("An error occurred in locating the queue. ("+MQE.Message+")","C#_Draw");
					this.Close();
					return;
				}
				if (mqlist.Length == 0)
				{
					MessageBox.Show("Unable to locate queue.","C#_DRAW");
					return;
				}
				qFriend = mqlist[0];
				this.Text= strName + " - Connected to " + textFriendName.Text;
			}
			else
			{
				
				if (textFriendComputer.Text == "")
				{
					MessageBox.Show("Please fill in computer name.","Missing value");
					return;
				}

				textFriendComputer.Text = textFriendComputer.Text.ToUpper();

				/*
				When working in direct mode, we must use private queues. We are unable
				to know whether a given private queue on another computer exists or not,
				so here we will just assume that it does. To make the application more robust,
				an acknowledgement queue should be created on the local computer, and
				a request for acknowledgement messages should be added to the messages sent.
				Then the application can notify the user when a negative acknoledgement (NACK)
				message is received.
				*/

				qFriend = new MessageQueue("FormatName:DIRECT=OS:" +
										   textFriendComputer.Text +
					                       @"\PRIVATE$\"+textFriendName.Text);
				this.Text = @".\PRIVATE$\"+strName + " - Connected to " +
							textFriendComputer.Text + @"\PRIVATE$\"+textFriendName.Text;
			}
			textChars.Enabled = true;
			picture1.Enabled = true;
			btnAttach.Enabled = false;
			return;
		}

		//
		// Repainting the display
		//
		private void picture1_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
		{
			int i;
			for (i=0;i<lines.Count;i++)
			{
				DrawLine((Util.line)lines[i],e.Graphics);
			}
		}
		private void btnConnect_Click(object sender, System.EventArgs e)
		{
			MessageQueue[] mqlist;
			MessageQueueCriteria mqCriteria;
			System.Windows.Forms.Cursor TempCursor;

			bDirectMode = radioDSDisabled.Checked;
			TempCursor = this.Cursor;
			this.Cursor = Cursors.WaitCursor;

			if (radioDSDisabled.Checked)
			{ // User wants to use a private queue (direct connection).
				try 
				{
					qMe=MessageQueue.Create(@".\private$\" + strName);
					qMe.Label = strName;
					qMe.Category = new System.Guid(guidDraw);

				}
				catch(MessageQueueException MQE)
				{
					qMe = new MessageQueue();
					qMe.Path = @".\private$\" + strName;
				}
			}
			else 
			{ // The user wants to use a public queue (standard mode)
			  // so query the direcotry service for a public queue with the
			  // user-specified name. 
				mqCriteria = new MessageQueueCriteria();
				mqCriteria.Label=strName;
				mqCriteria.Category = new System.Guid(guidDraw);
				try
				{
					mqlist = MessageQueue.GetPublicQueues(mqCriteria);
				}
				catch(MessageQueueException MQE)
				{	
					MessageBox.Show("An error occurred in locating the queue. ("+MQE.Message+")","C#_Draw");
					this.Close();
					return;
				}
				// If no such queue then create one
				if (mqlist.Length == 0) 
				{
					try
					{
						MessageQueue.Create(@".\"+strName);
					}
					catch(MessageQueueException MQE)
					{	
						MessageBox.Show("An error occurred in creating the queue. ("+MQE.Message+")","C#_Draw");
						this.Close();
						return;
					}
					qMe = new MessageQueue();
					qMe.Path = @".\"+strName;
					qMe.Label = strName;
					qMe.Category = new System.Guid(guidDraw);
				}
				else
				{
					qMe=mqlist[0];
				}
			}
			
			// Start receiving

			qMe.Formatter = new ActiveXMessageFormatter();
			qMe.ReceiveCompleted  += new ReceiveCompletedEventHandler(this.qMe_ReceiveCompleted);
			qMe.BeginReceive();

			groupConnectionMode.Visible = false;
			btnConnect.Visible=false;
			btnCancel.Visible=false;

			btnAttach.Visible=true;
			groupSendType.Visible=true;
			lblRemoteQueue.Visible=true;
			textFriendName.Visible=true;

			if (radioDSDisabled.Checked)
			{
				lblRemoteComputer.Visible=true;
				textFriendComputer.Visible=true;
			}
			
			this.Cursor = TempCursor;
			// Anyway, on the local computer, the connection to the Directory
			// service remains available
			lblDSConnection.Text = "MODE: Domain";
		}
		
		//
		// Message receiving handler
		//
		private void qMe_ReceiveCompleted(object sender, System.Messaging.ReceiveCompletedEventArgs e)
		{
			Util.line lineToDraw = new Util.line();
			MessageQueue mq = (MessageQueue)sender;
			System.Messaging.Message m = mq.EndReceive(e.AsyncResult);
			if (m.Body.ToString().Length==1)
			{ // Draw a char
				DrawChar(m.Body.ToString());
			}
			else
			{ // Draw a line
				HP.StringToLine(m.Body.ToString(),ref lineToDraw);
				AddLine(lineToDraw,picture1Graphics);
			}
			mq.BeginReceive();
		}
		private void btnCancel_Click(object sender, System.EventArgs e)
		{
			this.Close();
		}

		private void textFriendComputer_TextChanged(object sender, System.EventArgs e)
		{
			btnAttach.Enabled = true;
		}

		private void textFriendName_TextChanged(object sender, System.EventArgs e)
		{
			btnAttach.Enabled = true;
		}

		
	}
}


