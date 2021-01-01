--[[                  Stations
These are templates for space stations.
----------------------------------------------------------]]

template = ShipTemplate():setName("Small Station"):setLocaleName(_("Small Station")):setModel("space_station_4"):setType("station"):setClass("Station", "Standart station")
template:setDescription(_([[Stations of this size are often used as research outposts, listening stations, and security checkpoints. Crews turn over frequently in a small station's cramped accommodatations, but they are small enough to look like ships on many long-range sensors, and organized raiders sometimes take advantage of this by placing small stations in nebulae to serve as raiding bases. They are lightly shielded and vulnerable to swarming assaults.]]))
template:setHull(150)
template:setShields(300)
template:setRadarTrace("radartrace_smallstation.png")

template = ShipTemplate():setName("Medium Station"):setLocaleName(_("Medium Station")):setModel("space_station_3"):setType("station"):setClass("Station", "Standart station")
template:setDescription(_([[Large enough to accommodate small crews for extended periods of times, stations of this size are often trading posts, refuelling bases, mining operations, and forward military bases. While their shields are strong, concerted attacks by many ships can bring them down quickly.]]))
template:setHull(400)
template:setShields(800)
template:setRadarTrace("radartrace_mediumstation.png")

template = ShipTemplate():setName("Large Station"):setLocaleName(_("Large Station")):setModel("space_station_2"):setType("station"):setClass("Station", "Standart station")
template:setDescription(_([[These spaceborne communities often represent permanent bases in a sector. Stations of this size can be military installations, commercial hubs, deep-space settlements, and small shipyards. Only a concentrated attack can penetrate a large station's shields, and its hull can withstand all but the most powerful weaponry.]]))
template:setHull(500)
template:setShields(1000, 1000, 1000)
template:setRadarTrace("radartrace_largestation.png")

template = ShipTemplate():setName("Huge Station"):setLocaleName(_("Huge Station")):setModel("space_station_1"):setType("station"):setClass("Station", "Standart station")
template:setDescription(_([[The size of a sprawling town, stations at this scale represent a faction's center of spaceborne power in a region. They serve many functions at once and represent an extensive investment of time, money, and labor. A huge station's shields and thick hull can keep it intact long enough for reinforcements to arrive, even when faced with an ongoing siege or massive, perfectly coordinated assault.]]))
template:setHull(800)
template:setShields(1200, 1200, 1200, 1200)
template:setRadarTrace("radartrace_hugestation.png")

template = ShipTemplate():setName("Babylon Station"):setLocaleName(_("Babylon Station")):setModel("station_babylon"):setType("station"):setClass("Station", "Station")
template:setDescription([[ ]])
template:setHull(1200)
template:setShields(1200, 1200, 1200, 1200)
template:setRadarTrace("radartrace_hugestation.png")

template = ShipTemplate():setName("Tore Station"):setLocaleName(_("Tore Station")):setModel("station_tore"):setType("station"):setClass("Station", "Station"):setRecorded(false)
template:setDescription([[ ]])
template:setHull(500)
template:setShields(200, 200)
template:setRadarTrace("radartrace_hugestation.png")

template = ShipTemplate():setName("Commerce Station"):setLocaleName(_("Commerce Station")):setModel("station_commerce"):setType("station"):setClass("Station", "Station")
template:setDescription([[ .]])
template:setHull(600)
template:setShields(1200)
template:setRadarTrace("radartrace_mediumstation.png")
