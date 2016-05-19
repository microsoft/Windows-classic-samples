//-------------------------------------------------------------------------
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//  File: Serialization.cs
//  Ink Serialization Application
//
//  This program demonstrates how one can serialize and deserialize 
//  ink in various formats.
//
//  The application represents a form with fields for inputting first
//  name, last name and signature. It allows the user to save this
//  data as pure ISF (Ink Serialized Format) XML using base64 encoded
//  ISF or HTML which references GIF images "fortified" with ink.  It
//  is also possible to Load from the XML and ISF formats.  The ISF 
//  save/load uses extended properties to store the first name and last
//  name, whereas the XML and HTML save/load stores this information in
//  custom attributes.
//
//  This sample does not support loading from the HTML format, because
//  HTML is not a format suitable for storing structured data, such as
//  forms. Because the data is separated into multiple streams (name,
//  signature, etc.,) a format which preserves this separation such as
//  XML or another kind of database format is required.
//
//  HTML is very useful in a flow editing environment, such as a word
//  processing document. The HTML that is saved by this sample uses
//  fortified GIFs. These GIFs have ISF embedded within them, which
//  preserves the full fidelity of the ink. A word processing
//  application could save a document containing multiple types of
//  data, such as images, tables, formatted text and ink persisted
//  in an HTML format. This HTML would render without a hitch in
//  browsers which have no special abilities to understand ink.
//  However, when loaded into an application that is ink-aware, such
//  as another word processing application, the full fidelity of the
//  original ink is available and can be rendered at very high quality,
//  edited or used for recognition.
//
//  See the "Ink Interoperability" whitepaper, in this SDK, for more
//  information about persistence formats for ink.
//
//  The features used are: Load method, Save method, and extended properties
//
//--------------------------------------------------------------------------

using System;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using System.Text;
using System.IO;
using System.Xml;
using System.Runtime.InteropServices;

// The Ink namespace, which contains the Tablet PC Platform API
using Microsoft.Ink;

namespace Microsoft.Samples.TabletPC.Serialization
{
    public class SerializationForm : System.Windows.Forms.Form
    {

        #region Standard Template Code
        /// Required designer variable.
        private System.ComponentModel.Container components = null;
    
        private System.Windows.Forms.MainMenu MenuBar;
        private System.Windows.Forms.MenuItem FileMenu;
        private System.Windows.Forms.MenuItem NewMenu;
        private System.Windows.Forms.MenuItem OpenMenu;
        private System.Windows.Forms.MenuItem SaveAsMenu;
        private System.Windows.Forms.MenuItem SeperatorMenu;
        private System.Windows.Forms.MenuItem ExitMenu;

        private System.Windows.Forms.GroupBox Signature;
        private System.Windows.Forms.TextBox FirstNameBox;
        private System.Windows.Forms.TextBox LastNameBox;
        
        private System.Windows.Forms.Label FirstNameLabel;
        private System.Windows.Forms.Label LastNameLabel;
        #endregion

        // The one and only ink collector
        private InkCollector ic = null;

        // These strings identify the GUIDs for custom attributes
        private Guid FirstName = new Guid ("{29B2E996-57B6-4fda-BFFD-F54DBA4C35F9}");
        private Guid LastName = new Guid ("{8A448E7C-0F39-4cd9-BB52-71FDA3221674}");

        // Constructor
        public SerializationForm()
        {
            #region Standard Template Code
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            #endregion

            // Create the InkCollector and attach it to the signature GroupBox
            ic = new InkCollector(Signature.Handle);
            ic.Enabled = true;
        }

        #region Standard Template Code
        /// Clean up any resources being used.
        /// Windows Forms Designer template code
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
            this.MenuBar = new System.Windows.Forms.MainMenu();
            this.FileMenu = new System.Windows.Forms.MenuItem();
            this.NewMenu = new System.Windows.Forms.MenuItem();
            this.OpenMenu = new System.Windows.Forms.MenuItem();
            this.SaveAsMenu = new System.Windows.Forms.MenuItem();
            this.SeperatorMenu = new System.Windows.Forms.MenuItem();
            this.ExitMenu = new System.Windows.Forms.MenuItem();
            this.Signature = new System.Windows.Forms.GroupBox();
            this.FirstNameBox = new System.Windows.Forms.TextBox();
            this.LastNameBox = new System.Windows.Forms.TextBox();
            this.FirstNameLabel = new System.Windows.Forms.Label();
            this.LastNameLabel = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // MenuBar
            // 
            this.MenuBar.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                    this.FileMenu});
            // 
            // FileMenu
            // 
            this.FileMenu.Index = 0;
            this.FileMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                                     this.NewMenu,
                                                                                     this.OpenMenu,
                                                                                     this.SaveAsMenu,
                                                                                     this.SeperatorMenu,
                                                                                     this.ExitMenu});
            this.FileMenu.Text = "&File";
            // 
            // NewMenu
            // 
            this.NewMenu.Index = 0;
            this.NewMenu.Shortcut = System.Windows.Forms.Shortcut.CtrlN;
            this.NewMenu.Text = "&New";
            this.NewMenu.Click += new System.EventHandler(this.NewMenu_Click);
            // 
            // OpenMenu
            // 
            this.OpenMenu.Index = 1;
            this.OpenMenu.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
            this.OpenMenu.Text = "&Open...";
            this.OpenMenu.Click += new System.EventHandler(this.OpenMenu_Click);
            // 
            // SaveAsMenu
            // 
            this.SaveAsMenu.Index = 2;
            this.SaveAsMenu.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
            this.SaveAsMenu.Text = "Save &As...";
            this.SaveAsMenu.Click += new System.EventHandler(this.SaveAsMenu_Click);
            // 
            // SeperatorMenu
            // 
            this.SeperatorMenu.Index = 3;
            this.SeperatorMenu.Text = "-";
            // 
            // ExitMenu
            // 
            this.ExitMenu.Index = 4;
            this.ExitMenu.Text = "E&xit";
            this.ExitMenu.Click += new System.EventHandler(this.ExitMenu_Click);
            // 
            // Signature
            // 
            this.Signature.Anchor = (((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
                | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right);
            this.Signature.Location = new System.Drawing.Point(8, 48);
            this.Signature.Name = "Signature";
            this.Signature.Size = new System.Drawing.Size(456, 67);
            this.Signature.TabIndex = 1;
            this.Signature.TabStop = false;
            this.Signature.Text = "Signature";
            // 
            // FirstNameBox
            // 
            this.FirstNameBox.Location = new System.Drawing.Point(8, 24);
            this.FirstNameBox.Name = "FirstNameBox";
            this.FirstNameBox.Size = new System.Drawing.Size(200, 20);
            this.FirstNameBox.TabIndex = 2;
            this.FirstNameBox.Text = "";
            // 
            // LastNameBox
            // 
            this.LastNameBox.Anchor = ((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
                | System.Windows.Forms.AnchorStyles.Right);
            this.LastNameBox.Location = new System.Drawing.Point(216, 24);
            this.LastNameBox.Name = "LastNameBox";
            this.LastNameBox.Size = new System.Drawing.Size(248, 20);
            this.LastNameBox.TabIndex = 3;
            this.LastNameBox.Text = "";
            // 
            // FirstNameLabel
            // 
            this.FirstNameLabel.Location = new System.Drawing.Point(8, 8);
            this.FirstNameLabel.Name = "FirstNameLabel";
            this.FirstNameLabel.Size = new System.Drawing.Size(100, 16);
            this.FirstNameLabel.TabIndex = 4;
            this.FirstNameLabel.Text = "First Name";
            // 
            // LastNameLabel
            // 
            this.LastNameLabel.Location = new System.Drawing.Point(216, 8);
            this.LastNameLabel.Name = "LastNameLabel";
            this.LastNameLabel.Size = new System.Drawing.Size(100, 16);
            this.LastNameLabel.TabIndex = 5;
            this.LastNameLabel.Text = "Last Name";
            // 
            // SerializationForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(472, 121);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.LastNameLabel,
                                                                          this.FirstNameLabel,
                                                                          this.LastNameBox,
                                                                          this.FirstNameBox,
                                                                          this.Signature});
            this.Menu = this.MenuBar;
            this.Name = "SerializationForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Ink Serialization Sample Application";
            this.ResumeLayout(false);

        }
        #endregion

        #region Standard Template Code
        /// The main entry point for the application.
        [STAThread]
        static void Main() 
        {
            Application.Run(new SerializationForm());
        }
        #endregion

        // If the user hits the new menu, clear the document
        private void NewMenu_Click(object sender, System.EventArgs e)
        {
            // Erase the ink
            ic.Ink.DeleteStrokes();
            Signature.Invalidate();

            // Clear the text boxes
            FirstNameBox.Text = "";
            LastNameBox.Text = "";
        }

        // If the user hits the exit menu, exit the application
        private void ExitMenu_Click(object sender, System.EventArgs e)
        {
            ic.Enabled = false;
            this.Dispose();
        }

        // This function handles the Save As.. command. It allows the user
        // to specify a filename and location, as well as the type of format
        // to save in. It then calls the appropriate helper function.
        private void SaveAsMenu_Click(object sender, System.EventArgs e)
        {
            /// Create the SaveFileDialog, which presents a standard Windows
            /// Save dialog to the user.
            SaveFileDialog saveDialog = new SaveFileDialog();

            /// Set the filter to suggest our recommended extensions
            saveDialog.Filter =
                "Ink Serialized Format files (*.isf)|*.isf|XML files (*.xml)|*.xml|" +
                "HTML files (*.htm)|*.htm";
 
            /// If the dialog exits and the user didn't choose Cancel
            if(saveDialog.ShowDialog() == DialogResult.OK)
            {
                /// Create a stream which will be used to save data to the output file
                /// Attempt to Open the file with read/write permission
                using (FileStream myStream = new FileStream(saveDialog.FileName, FileMode.Create, FileAccess.ReadWrite))
                {
                    // Put the filename in a more canonical format
                    String filename = saveDialog.FileName.ToLower();

                    // Get a version of the filename without an extension
                    // This will be used for saving the associated image
                    String extensionlessFilename = Path.GetFileNameWithoutExtension(filename);

                    // Get the extension of the file 
                    String extension = Path.GetExtension(filename);                     

                    // Use the extension to determine what form to save the data in
                    switch (extension) 
                    {
                        case ".xml":
                            SaveXML(myStream);
                            break;

                            /// The two HTML cases require a filename for saving associated images
                        case ".htm": case ".html":
                            SaveHTML(myStream, extensionlessFilename);
                            break;

                            /// If unfamiliar with the extension, use ISF, the most "native format"
                        case ".isf": default:
                            SaveISF(myStream);
                            break;
                    }
                }
            } // End if user chose OK from dialog 
        }

        // This function saves the form in ISF format.
        // It uses ExtendedProperties to preserve the first and last names.
        // ExtendedProperties are an easy way to store non-ink data within an
        // ink object. In this case, there is no outer format which contains the
        // ink, so the only place to store the names is within the ink object itself.
        private void SaveISF(Stream s) 
        {
            byte[] isf;
            
            // This is the ink object which will be serialized
            ExtendedProperties inkProperties = ic.Ink.ExtendedProperties;

            // Store the name fields in the ink object
            // These fields will roundtrip through the ISF format
            // Ignore empty fields since strictly empty strings 
            //       cannot be stored in ExtendedProperties.
            if (FirstNameBox.Text.Length > 0)
            {
                inkProperties.Add(FirstName, FirstNameBox.Text);
            }
            if (LastNameBox.Text.Length > 0)
            {
                inkProperties.Add(LastName, LastNameBox.Text);
            }

            // Perform the serialization
            isf = ic.Ink.Save(PersistenceFormat.InkSerializedFormat);

            // If the first and last names were added as extended
            // properties to the ink, remove them - these properties
            // are only used for the save and there is no need to
            // keep them around on the ink object.
            if (inkProperties.DoesPropertyExist(FirstName))
            {
                inkProperties.Remove(FirstName);
            }
            if (inkProperties.DoesPropertyExist(LastName))
            {
                inkProperties.Remove(LastName);
            }

            // Write the ISF to the stream
            s.Write(isf,0,isf.Length);
        }

        // This function saves the form in XML format.
        // It uses the base64 encoded version of the ink, which is most suitable
        // for representation in XML. The names are stored as XML fields, rather
        // than custom properties, so that these properties can be most easily
        // accessible if the XML is saved in a database.
        private void SaveXML(Stream s) 
        {
            // This object will encode our byte data to a UTF8 string
            UTF8Encoding utf8 = new UTF8Encoding();
            
            byte[] base64ISF_bytes;
            string base64ISF_string;

            // Create a new XmlTextWriter.
            XmlTextWriter xwriter = new XmlTextWriter(s, System.Text.Encoding.UTF8);
                        
            // Write the beginning of the document including the 
            // document declaration. 
            xwriter.WriteStartDocument();

            // Write the beginning of the "data" element. This is 
            // the opening tag to our data 
            xwriter.WriteStartElement("SerializationSampleData");

            // Get the base64 encoded ISF
            base64ISF_bytes = ic.Ink.Save(PersistenceFormat.Base64InkSerializedFormat);

            // Ink.Save returns a null terminated byte array. The encoding of the null
            // character generates a control sequence when it is UTF8 encoded. This
            // sequence is invalid in XML. Therefore, it is necessary to remove the 
            // null character before UTF8 encoding the array.
            // The following loop finds the index of the first non-null byte in the byte 
            // array returned by the Ink.Save method.
            int countOfBytesToConvertIntoString = base64ISF_bytes.Length - 1;
            for(; countOfBytesToConvertIntoString >= 0; --countOfBytesToConvertIntoString)
            {
                // Break the loop if the byte at the index is non-null.
                if(0 != base64ISF_bytes[countOfBytesToConvertIntoString])
                    break;
            }

            // Convert the index into count by incrementing it.
            countOfBytesToConvertIntoString++;

            // Convert it to a String
            base64ISF_string = utf8.GetString(base64ISF_bytes, 0, countOfBytesToConvertIntoString);

            // Write the ISF containing node to the XML
            xwriter.WriteElementString("Ink", base64ISF_string);

            // Write the text date from the form
            // Note that the names are stored as XML fields, rather
            // than custom properties, so that these properties can 
            // be most easily accessible if the XML is saved in a database.
            xwriter.WriteElementString("FirstName",FirstNameBox.Text);
            xwriter.WriteElementString("LastName",LastNameBox.Text);

            //End the "data" element.
            xwriter.WriteEndElement();

            //End the document
            xwriter.WriteEndDocument();

            //Close the xml document.
            xwriter.Close();
        }

        // This function saves the form in HTML format.
        // It also creates an ink fortified GIF which is referenced by the HTML.
        // A fortified GIF is an image file that has ISF embedded inside it. To an
        // ink-unaware application, it is simply an image, but to an ink-aware
        // application, all of the original ink data is available.
        //
        // Please refer to the "Ink Interoperability" whitepaper for more information
        // on fortified GIFs.
        private void SaveHTML(Stream s, String nameBase) 
        {

            // It is not possible to save an empty .gif, so it 
            // is first necessary to ensure that the ink has a bounding
            // box (i.e. the ink is not empty).  The bounding box is 
            // used for this check instead of checking whether the stroke
            // count is zero, since the ink could still be empty if the
            // strokes don't contain any points.
            if (ic.Ink.Strokes.GetBoundingBox().IsEmpty)
            {
                MessageBox.Show("Unable to save empty ink in HTML persistence format.");
            }
            else
            {

                byte[] fortifiedGif = null;
                String html;
                byte[] htmlBytes;

                // This object will encode our byte data to a UTF8 string
                UTF8Encoding utf8 = new UTF8Encoding();
            
                // Create a directory to store the fortified GIF which also contains ISF
                // and open the file for writing
                Directory.CreateDirectory(nameBase + "_files");
                using (FileStream gifFile = File.OpenWrite(nameBase + "_files\\signature.gif"))
                {
                
                    // Generate the fortified GIF represenation of the ink
                    fortifiedGif = ic.Ink.Save(PersistenceFormat.Gif);

                    // Write and close the gif file
                    gifFile.Write(fortifiedGif, 0, fortifiedGif.Length);
                }

                // Create the HTML output
                // Note that the names are stored in HTML tags, rather
                // than custom properties, so that these properties can 
                // be easily retrieved from the HTML.
                html =
                    "<HTML><HEAD></HEAD><BODY>" +
    
                    "<P>First Name: " + 
                    FirstNameBox.Text +         
                    " &nbsp;&nbsp;&nbsp;Last Name: " + 
                    LastNameBox.Text +          
                    "</P>" +

                    "<P><IMG src='" +
                    nameBase + "_files\\signature.gif" +
                    "' /></P>" + 

                    "</BODY></HTML>";

                // Convert the HTML to a byte array for writing to the stream
                htmlBytes = utf8.GetBytes(html);

                // Write the HTML to the stream
                s.Write(htmlBytes, 0, htmlBytes.Length);
            }
        }

        // This function handles the Open... command. It will determine the type of ink which is being
        // opened and call the appropriate helper routine.
        private void OpenMenu_Click(object sender, System.EventArgs e)
        {
            /// Create the OpenFileDialog, which presents a standard Windows
            /// dialog to the user.
            OpenFileDialog openDialog = new OpenFileDialog();

            /// Set the filter to suggest our recommended extensions
            openDialog.Filter =
                "All Known Files (*.isf; *.xml)|*.isf; *.xml|Ink Serialized Format files (*.isf)|*.isf|" +
                "XML files (*.xml)|*.xml|All Files (*.*)|*.*";
 
            /// If the dialog exits and the user didn't choose Cancel
            if(openDialog.ShowDialog() == DialogResult.OK)
            {
                /// Create a stream which will be used to load data from the output file
                /// Attempt to Open the file with read only permission
                using( FileStream myStream = new FileStream( openDialog.FileName, FileMode.Open, FileAccess.Read ) )
                {
                    // Put the filename in a more canonical format
                    String filename = openDialog.FileName.ToLower();

                    // Get the extension of the file 
                    String extension = filename.Substring(filename.LastIndexOf('.'));                       

                        // verify that the user is not actively inking
                    if (!ic.CollectingInk)
                    {
                        // Use the extension to determine what form to save the data in
                        switch (extension) 
                        {
                            case ".isf":
                                LoadISF(myStream);
                                break;

                            case ".xml":
                                LoadXML(myStream);
                                break;

                                // If unfamiliar with the extension, assume ISF, the most "native format"
                            default:
                                LoadISF(myStream);
                                break;
                        }
                    }
                    else 
                    {
                        // If ink collection is enabled, the active ink cannot be changed
                        MessageBox.Show("Unable to load ink while actively inking. Please try again.", 
                            "Serialization", 
                            MessageBoxButtons.OK);
                    }
                }
            }
        }

        // This function will load ISF into the ink object.
        // It will also pull the data stored in the object's extended properties and
        // repopulate the text boxes.
        private void LoadISF(Stream s) 
        {
            Microsoft.Ink.Ink loadedInk = new Microsoft.Ink.Ink();
            byte[] isfBytes = new byte[s.Length];

            // read in the ISF
            s.Read(isfBytes, 0, (int) s.Length);

            // load the ink into a new ink object
            // once an ink object has been "dirtied" it can never load ink again
            loadedInk.Load(isfBytes);

            // temporarily disable the ink collector and swap ink objects
            // We checked the InkCollector.CollectingInk property to verify
            // the user has stopped inking. Otherwise, Enabled will throw
            // InvalidOperationException.
            ic.Enabled = false;

            ic.Ink = loadedInk;
            ic.Enabled = true;
 
            // Repaint the inkable region
            Signature.Invalidate();

            ExtendedProperties inkProperties = ic.Ink.ExtendedProperties;

            // Get the raw data out of this stroke's extended
            // properties list, using our previously defined 
            // Guid as a key to the extended property we want.
            // Since the save method stored the first and last
            // name information as extended properties, we can
            // remove this information now that the load is complete.
            if (inkProperties.DoesPropertyExist(FirstName))
            {
                FirstNameBox.Text = (String) inkProperties[FirstName].Data;
                inkProperties.Remove(FirstName);
            }
            else
            {
                FirstNameBox.Text = String.Empty;
            }

            if (inkProperties.DoesPropertyExist(LastName))
            {
                LastNameBox.Text = (String) inkProperties[LastName].Data;
                inkProperties.Remove(LastName);
            }
            else
            {
                LastNameBox.Text = String.Empty;
            }
        }

        // This function will load XML into the ink object.
        // It will repopulate the text boxes using the data stored in the XML.
        private void LoadXML(Stream s) 
        {
            // This object will encode our byte data to a UTF8 string
            UTF8Encoding utf8 = new UTF8Encoding();
            
            XmlDocument xd = new XmlDocument();
            XmlNodeList nodes;
            Microsoft.Ink.Ink loadedInk = new Microsoft.Ink.Ink();

            // Load the XML data into an XMLDocument object
            xd.Load(s);

            // Get the data in the ink node
            nodes = xd.GetElementsByTagName("Ink");

            // load the ink into a new ink object
            // once an ink object has been "dirtied" it can never load ink again
            if (0 != nodes.Count)
            {
                loadedInk.Load(utf8.GetBytes(nodes[0].InnerXml));
            }

            // temporarily disable the ink collector and swap ink objects
            // We checked the InkCollector.CollectingInk property to verify
            // the user has stopped inking. Otherwise, Enabled will throw
            // InvalidOperationException.
            ic.Enabled = false;

            ic.Ink = loadedInk;
            ic.Enabled = true;

            // Repaint the inkable region
            Signature.Invalidate();
                    
            // Get the data in the FirstName node
            nodes = xd.GetElementsByTagName("FirstName");
            if (0 != nodes.Count)
            {
                FirstNameBox.Text = nodes[0].InnerXml;
            }
            else
            {
                FirstNameBox.Text = String.Empty;
            }

            // Get the data in the LastName node
            nodes = xd.GetElementsByTagName("LastName");
            if (0 != nodes.Count)
            {
                LastNameBox.Text = nodes[0].InnerXml;
            }
            else
            {
                LastNameBox.Text = String.Empty;
            }
        }
    }
}
