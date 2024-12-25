<?xml version="1.0" encoding="UTF-8"?>
<tileset version="1.9" tiledversion="1.8.5" name="AutoMap Rules" tilewidth="32" tileheight="32" tilecount="5" columns="5" tilerendersize="grid" fillmode="preserve-aspect-fit">
 <image source="automap-tiles.svg" width="160" height="32"/>
 <tile id="3">
  <properties>
   <property name="MatchType" value="Empty"/>
  </properties>
 </tile>
 <tile id="1">
  <properties>
   <property name="MatchType" value="Ignore"/>
  </properties>
 </tile>
 <tile id="2">
  <properties>
   <property name="MatchType" value="NonEmpty"/>
  </properties>
 </tile>
 <tile id="4">
  <properties>
   <property name="MatchType" value="Other"/>
  </properties>
 </tile>
 <tile id="0">
  <properties>
   <property name="MatchType" value="Negate"/>
  </properties>
 </tile>
</tileset>
