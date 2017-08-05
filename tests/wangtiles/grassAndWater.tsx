<?xml version="1.0" encoding="UTF-8"?>
<tileset name="GrassAndWater" tilewidth="64" tileheight="64" tilecount="24" columns="4">
 <image source="grass_and_water.png" width="256" height="384"/>
 <terraintypes>
  <terrain name="New Terrain" tile="0"/>
  <terrain name="New Terrain" tile="23"/>
 </terraintypes>
 <tile id="0" terrain="0,0,0,0"/>
 <tile id="1" terrain="0,0,0,0"/>
 <tile id="2" terrain="0,0,0,0"/>
 <tile id="3" terrain="0,0,0,0"/>
 <tile id="4" terrain="0,0,0,1"/>
 <tile id="5" terrain="0,0,1,0"/>
 <tile id="6" terrain="1,0,0,0"/>
 <tile id="7" terrain="0,1,0,0"/>
 <tile id="8" terrain="0,1,1,1"/>
 <tile id="9" terrain="1,0,1,1"/>
 <tile id="10" terrain="1,1,1,0"/>
 <tile id="11" terrain="1,1,0,1"/>
 <tile id="12" terrain="0,0,1,1"/>
 <tile id="13" terrain="1,0,1,0"/>
 <tile id="14" terrain="1,1,0,0"/>
 <tile id="15" terrain="0,1,0,1"/>
 <tile id="20" terrain="0,1,1,0"/>
 <tile id="21" terrain="1,0,0,1"/>
 <tile id="22" terrain="1,1,1,1"/>
 <tile id="23" terrain="1,1,1,1"/>
 <wangsets>
  <wangset name="Grass and Water" edges="1" corners="2" tile="6">
   <wangcornercolor name="grass" index="1" r="0" g="170" b="0" imageTile="3" probability="1"/>
   <wangcornercolor name="water" index="2" r="0" g="85" b="255" imageTile="23" probability="1"/>
   <wangtile tileid="0" wangid="0x10101010"/>
   <wangtile tileid="1" wangid="0x10101010"/>
   <wangtile tileid="2" wangid="0x10101010"/>
   <wangtile tileid="3" wangid="0x10101010"/>
   <wangtile tileid="4" wangid="0x10102010"/>
   <wangtile tileid="5" wangid="0x10201010"/>
   <wangtile tileid="6" wangid="0x20101010"/>
   <wangtile tileid="7" wangid="0x10101020"/>
   <wangtile tileid="8" wangid="0x10202020"/>
   <wangtile tileid="9" wangid="0x20202010"/>
   <wangtile tileid="10" wangid="0x20201020"/>
   <wangtile tileid="11" wangid="0x20102020"/>
   <wangtile tileid="12" wangid="0x10202010"/>
   <wangtile tileid="13" wangid="0x20201010"/>
   <wangtile tileid="14" wangid="0x20101020"/>
   <wangtile tileid="15" wangid="0x10102020"/>
   <wangtile tileid="16" wangid="0x10202010"/>
   <wangtile tileid="17" wangid="0x20201010"/>
   <wangtile tileid="18" wangid="0x20101020"/>
   <wangtile tileid="19" wangid="0x10102020"/>
   <wangtile tileid="20" wangid="0x10201020"/>
   <wangtile tileid="21" wangid="0x20102010"/>
   <wangtile tileid="22" wangid="0x20202020"/>
   <wangtile tileid="23" wangid="0x20202020"/>
  </wangset>
 </wangsets>
</tileset>
