<xsl:stylesheet version="1.0" xmlns:xsl='http://www.w3.org/1999/XSL/Transform'
	 xmlns:java-lib="http://xml.apache.org/xslt/java/java.net.URLEncoder">
<xsl:param name="curName"/>

<xsl:template match="/">  

  <xsl:for-each select="/catalog//*">
  <xsl:if test="@id=$curName">
	<TABLE border = "3" bordercolor = "CCCC99">
	<TR><TD><xsl:attribute name="id">oTD<xsl:value-of select="@id"/></xsl:attribute>
    <xsl:apply-templates select="."/> 
	</TD></TR>
	</TABLE>
  </xsl:if>
  </xsl:for-each>  

</xsl:template>

<xsl:template match="subdir"> 
  <xsl:for-each select="node()">
  <xsl:choose> 

  <xsl:when test="@pictureType[.='folder']">
<DIV> <xsl:attribute name="id">oDivExt<xsl:value-of select="@id"/></xsl:attribute>
      
	<IMG src="pict/closed.bmp" class="folder">
  		 <xsl:attribute name="CatID"><xsl:value-of select="@id"/></xsl:attribute> 
	</IMG>

	<SPAN class="catalogInfo">
	 <xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
 	<xsl:value-of select="@name"/>
	 </SPAN>

    	<DIV><xsl:attribute name="id">oDiv<xsl:value-of select="@id"/></xsl:attribute></DIV>
 </DIV>
  </xsl:when>
  
<xsl:when test="@pictureType[.='folder_paper']">
<DIV class="selDir"> <xsl:attribute name="id">oDivExt<xsl:value-of select="@id"/></xsl:attribute>      
	<IMG src="pict/folder_paper.bmp" class="folder">
  		 <xsl:attribute name="CatID"><xsl:value-of select="@id"/></xsl:attribute> 
		<xsl:attribute name="IsRoot">true</xsl:attribute>
	</IMG>
	<SPAN class="catalogInfo, paper">
	 <xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
        <xsl:attribute name="name"> <xsl:value-of select="@name"/></xsl:attribute>
 	<xsl:value-of select="@name"/>
	 </SPAN>
    	<DIV><xsl:attribute name="id">oDiv<xsl:value-of select="@id"/></xsl:attribute> </DIV>
 </DIV>
   </xsl:when>

<xsl:when test="@pictureType[.='envpaper']">
	<DIV class="selDir"> <xsl:attribute name="id">oDivExt<xsl:value-of select="@id"/></xsl:attribute>      
	<IMG src="pict/envpaper.bmp" class="folder">
  		 <xsl:attribute name="CatID">
         	 <xsl:value-of select="@id"/>
   		</xsl:attribute> 
		<xsl:attribute name="IsRoot">true</xsl:attribute>
	</IMG>
	<SPAN class="catalogInfo, paper">
	 <xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
	<xsl:attribute name="title"> <xsl:value-of select="@info"/></xsl:attribute>
        <xsl:attribute name="name"> <xsl:value-of select="@name"/></xsl:attribute>
 	<xsl:value-of select="@name"/>
	 </SPAN>
    	<DIV> <xsl:attribute name="id">oDiv<xsl:value-of select="@id"/></xsl:attribute> </DIV>
 	</DIV>
 </xsl:when>


  <xsl:otherwise>
 <DIV class="selDir"> <xsl:attribute name="id">oDivExt<xsl:value-of select="@id"/></xsl:attribute>
   <IMG SRC="pict/paper.bmp" > </IMG>  
     <SPAN class="catalogInfo, paper">
      <xsl:attribute name="title"> <xsl:value-of select="@info"/></xsl:attribute>
      <xsl:attribute name="name"> <xsl:value-of select="@name"/></xsl:attribute>
      <xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
     <xsl:value-of select="@name"/>     
      </SPAN>   
  </DIV> 
 
  </xsl:otherwise>
  </xsl:choose>  
  </xsl:for-each>  
</xsl:template>
</xsl:stylesheet>