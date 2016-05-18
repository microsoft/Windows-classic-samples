<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">

<!-- TOC.XSL -->

<xsl:template match="/">

	<HTML>
	<HEAD>

	<STYLE TYPE="text/css"><xsl:comment>

		BODY { font-family:verdana; font-size:70%; }
		H1 { font-size:120%; font-style:italic; }
	
		UL { margin-left:0px; margin-bottom:5px; }
		LI UL { display:none; margin-left:16px; }
		LI { font-weight:bold; list-style-type:square; cursor:default; }
		LI.clsHasKids { list-style-type:none; }
		LI.clsHasKids SPAN { cursor:hand; }
	
		A:link, A:visited, A:active { font-weight:normal; color:navy; }
		A:hover { text-decoration:none; }
	
	</xsl:comment></STYLE>

	<SCRIPT LANGUAGE="javascript"><xsl:comment><![CDATA[
	
		function GetChildElem(eSrc,sTagName)
		{
			var cKids = eSrc.children;
			for (var i=0;i<cKids.length;i++)
			{
				if (sTagName == cKids[i].tagName) return cKids[i];
			}
			return false;
		}

		function List_click()
		{
			var eSrc = window.event.srcElement;
			if ("SPAN" == eSrc.tagName && "clsHasKids" == eSrc.parentElement.className)
			{
				var eChild = GetChildElem(eSrc.parentElement,"UL");
				eChild.style.display = ("block" == eChild.style.display ? "none" : "block");
			}
		}

		function List_over()
		{
			var eSrc = window.event.srcElement;
			if ("SPAN" == eSrc.tagName && "clsHasKids" == eSrc.parentElement.className)
			{
				eSrc.style.color = "maroon";
			}
		}

		function List_out()
		{
			var eSrc = window.event.srcElement;
			if ("SPAN" == eSrc.tagName && "clsHasKids" == eSrc.parentElement.className)
			{
				eSrc.style.color = "";
			}
		}

		function ShowAll(sTagName)
		{
			var cElems = document.all.tags(sTagName);
			var iNumElems = cElems.length;
			for (var i=1;i<iNumElems;i++) cElems[i].style.display = "block";
		}

		function HideAll(sTagName)
		{
			var cElems = document.all.tags(sTagName);
			var iNumElems = cElems.length;
			for (var i=1;i<iNumElems;i++) cElems[i].style.display = "none";
		}

	]]>//</xsl:comment></SCRIPT>

	</HEAD>
	<BODY>

	<hr></hr>
	<!-- GENERATE SHOWALL AND HIDEALL BUTTONS -->

	<BUTTON ONCLICK="ShowAll('UL')">Show All</BUTTON>
	<BUTTON ONCLICK="HideAll('UL')">Hide All</BUTTON>
	<!-- BUILD LIST -->

	<UL ONMOUSEOVER="List_over();" ONMOUSEOUT="List_out();" ONCLICK="List_click();">
		<xsl:apply-templates select="TOPICLIST/TOPICS" />	
	</UL>
	<hr></hr>
	
	</BODY>
	</HTML>

</xsl:template>

<xsl:template match="TOPICS">
	<LI CLASS="clsHasKids"><SPAN><xsl:value-of select="@TYPE" /></SPAN>
	<UL>
	<xsl:for-each select="TOPIC">
		<LI>
			<A TARGET="fraContent">
				<xsl:attribute name="HREF"><xsl:value-of select="URL" /></xsl:attribute>
				<xsl:value-of select="TITLE" />
			</A>
		</LI>
	</xsl:for-each>
	<xsl:apply-templates select="TOPICS" />
	</UL>
	</LI>
</xsl:template>

</xsl:stylesheet>