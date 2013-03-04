return {
  version = "1.1",
  luaversion = "5.1",
  orientation = "orthogonal",
  width = 40,
  height = 13,
  tilewidth = 40,
  tileheight = 40,
  properties = {},
  tilesets = {
    {
      name = "grass2rock",
      firstgid = 1,
      tilewidth = 40,
      tileheight = 40,
      spacing = 0,
      margin = 0,
      image = "grass2rock.png",
      imagewidth = 640,
      imageheight = 480,
      properties = {},
      tiles = {
        {
          id = 39,
          properties = {
            ["grass"] = "true"
          }
        }
      }
    },
    {
      name = "tree",
      firstgid = 193,
      tilewidth = 120,
      tileheight = 160,
      spacing = 0,
      margin = 0,
      image = "tree.png",
      imagewidth = 120,
      imageheight = 160,
      properties = {},
      tiles = {}
    }
  },
  layers = {
    {
      type = "tilelayer",
      name = "Tile Layer 1",
      x = 0,
      y = 0,
      width = 40,
      height = 13,
      visible = true,
      opacity = 1,
      properties = {},
      encoding = "lua",
      data = {
        98, 104, 68, 70, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 66, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 70,
        40, 98, 100, 102, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 66, 68, 68, 68, 68, 68, 106, 100, 104, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 106, 102,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 98, 104, 68, 68, 106, 100, 102, 40, 98, 104, 68, 68, 68, 68, 68, 106, 100, 104, 68, 68, 68, 74, 38,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 98, 104, 68, 70, 40, 40, 40, 34, 72, 68, 106, 100, 100, 100, 102, 34, 72, 68, 68, 106, 100, 102,
        34, 38, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 66, 68, 70, 40, 40, 40, 66, 68, 68, 74, 36, 38, 40, 40, 66, 68, 68, 68, 74, 38, 40,
        98, 102, 34, 38, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 98, 104, 74, 38, 40, 40, 66, 68, 68, 68, 68, 70, 40, 34, 72, 68, 68, 68, 68, 70, 40,
        40, 34, 72, 70, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 34, 38, 66, 68, 70, 40, 34, 72, 68, 106, 100, 100, 102, 34, 72, 68, 68, 106, 104, 68, 70, 40,
        40, 98, 100, 102, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 34, 72, 74, 72, 106, 102, 34, 72, 68, 106, 102, 40, 40, 34, 72, 68, 68, 68, 74, 72, 106, 102, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 34, 36, 36, 36, 72, 106, 100, 104, 74, 38, 98, 104, 68, 70, 40, 34, 36, 72, 68, 68, 68, 68, 68, 106, 102, 40, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 34, 72, 106, 100, 100, 100, 102, 40, 98, 104, 70, 40, 98, 100, 102, 34, 72, 68, 68, 68, 68, 68, 106, 100, 102, 34, 38, 40,
        40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 34, 72, 106, 102, 40, 40, 40, 40, 40, 40, 98, 102, 40, 34, 36, 36, 72, 68, 68, 68, 68, 106, 100, 102, 40, 40, 98, 102, 40,
        40, 34, 38, 40, 40, 40, 40, 40, 40, 40, 40, 66, 106, 102, 40, 40, 40, 40, 40, 40, 40, 34, 36, 38, 66, 68, 68, 68, 68, 68, 68, 68, 70, 40, 40, 40, 34, 38, 34, 38,
        34, 72, 74, 38, 40, 40, 40, 40, 40, 40, 34, 72, 70, 40, 40, 40, 40, 40, 34, 36, 36, 72, 68, 74, 72, 68, 68, 68, 68, 68, 68, 68, 74, 36, 36, 36, 72, 74, 72, 70
      }
    },
    {
      type = "objectgroup",
      name = "Object Layer 1",
      visible = true,
      opacity = 1,
      properties = {},
      objects = {
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 146,
          y = 412,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 237,
          y = 442,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 197,
          y = 522,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 302,
          y = 425,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 683,
          y = 276,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 227,
          y = 372,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 173,
          y = 316,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 309,
          y = 175,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 393,
          y = 198,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 608,
          y = 276,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 353,
          y = 160,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 641,
          y = 230,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        },
        {
          name = "",
          type = "",
          shape = "rectangle",
          x = 515,
          y = 181,
          width = 0,
          height = 0,
          gid = 193,
          visible = true,
          properties = {}
        }
      }
    }
  }
}
