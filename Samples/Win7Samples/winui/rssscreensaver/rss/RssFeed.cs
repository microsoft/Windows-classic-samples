// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) 2006 Microsoft Corporation. All rights reserved.

using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Xml;
using Microsoft.Feeds.Interop;

namespace Microsoft.Samples.RssPlatform.ScreenSaver.Rss
{
    /// <summary>
    /// Representation of an RSS element in a RSS 2.0 XML document
    /// </summary>
    public class RssFeed : UI.IItem
    {
        private readonly string title;
        private readonly string link;
        private readonly string description;
        private readonly string path;
        private readonly DateTime lastWriteTime;
        private List<RssItem> items;

        public string Title { get { return title; } }
        public string Link { get { return link; } }
        public string Description { get { return description; } }
        public string Path { get { return path; } }
        public DateTime LastWriteTime { get { return lastWriteTime; } }
        public IList<RssItem> Items { get { return items.AsReadOnly(); } }

        /// <summary>
        /// Private constructor to be used with factory pattern.  
        /// </summary>
        /// <exception cref="System.Xml.XmlException">Occurs when the XML is not well-formed.</exception>
        /// <param name="xmlNode">An XML block containing the RSSFeed content.</param>
        private RssFeed(XmlNode xmlNode)
        {
            XmlNode channelNode = xmlNode.SelectSingleNode("rss/channel");
            items = new List<RssItem>();
            title = channelNode.SelectSingleNode("title").InnerText;
            link = channelNode.SelectSingleNode("link").InnerText;
            description = channelNode.SelectSingleNode("description").InnerText;

            XmlNodeList itemNodes = channelNode.SelectNodes("item");
            foreach (XmlNode itemNode in itemNodes)
            {
                RssItem rssItem = new RssItem(itemNode);
                
                // only add items that have enclosures
                if (rssItem.Enclosure != null)
                    items.Add(rssItem);
            }
        }

        private RssFeed(IFeed feed)
        {
            items = new List<RssItem>();
            title = feed.Title;
            link = feed.Link;
            description = feed.Description;
            path = feed.Path;
            lastWriteTime = feed.LastWriteTime;

            foreach (IFeedItem item in (IFeedsEnum)feed.Items)
            {
                RssItem rssItem =  new RssItem(item);

                // only add items that have enclosures
                if (rssItem.Enclosure != null)
                    items.Add(rssItem);
            } 
        }
        
        /// <summary>
        /// Factory that constructs RSSFeed objects from a uri pointing to a valid RSS 2.0 XML file.
        /// </summary>
        /// <exception cref="System.Net.WebException">Occurs when the uri cannot be located on the web.</exception>
        /// <param name="uri">The URL to read the RSS feed from.</param>
        /// <example>
        /*
         *  try
            {
                Uri url = new Uri("http://localhost/feed.rss");
                RssFeed.FromUri(url);
            }
            catch (UriFormatException ex)
            {
                MessageBox.Show(ex.Message, "Not a valid Url", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            catch (System.Net.WebException ex)
            {
                MessageBox.Show(ex.Message, "Failed to get Url", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            catch (System.Xml.XmlException ex)
            {
                MessageBox.Show(ex.Message, "Not a valid RSS feed.", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            MessageBox.Show("Valid RSS feed.", "Valid RSS feed.", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
         */
        /// </example>
        public static RssFeed FromUri(Uri uri)
        {
            XmlDocument xmlDoc;
            WebClient webClient = new WebClient();
            using (Stream rssStream = webClient.OpenRead(uri))
            {
                TextReader textReader = new StreamReader(rssStream);
                XmlTextReader reader = new XmlTextReader(textReader);
                xmlDoc = new XmlDocument();
                xmlDoc.Load(reader);
            }
            return new RssFeed(xmlDoc);
        }

        /// <summary>
        /// Factory to construct the RSSFeed object from the Windows RSS Platform API 
        /// </summary>
        /// <param name="feed">The Common Feed List feed object</param>
        /// <returns>An initialized RSSFeed object</returns>
        internal static RssFeed FromApi(IFeed feed)
        {
            RssFeed rssFeed = null;
            
            // Skip this feed if there are not items.
            if (feed != null
                && ((IFeedsEnum)feed.Items).Count > 0)
                rssFeed = new RssFeed(feed);
            return rssFeed;
        }

        /// <summary>
        /// Factory that constructs RssFeed objects from the text of an RSS 2.0 XML file.
        /// </summary>
        /// <param name="rssText">A string containing the XML for the RSS feed.</param>
        public static RssFeed FromText(string rssText)
        {
            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.LoadXml(rssText);
            return new RssFeed(xmlDoc);
        }
    }
}