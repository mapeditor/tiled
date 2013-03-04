local map = dofile "backgroundmap.lua"
local layer = map.layers[1]
local tileset = map.tilesets[1]

local function gid_at(x, y)
    return layer.data[x + layer.width * (y - 1)]
end

local sections = {}

for y=1,layer.height do
    local section = {
        gid = gid_at(1, y),
        x = 1,
        y = y,
        length = 1,
    }

    for x=2,layer.width do
        local gid = gid_at(x, y)

        if section.gid ~= gid then
            sections[#sections + 1] = section

            section = {
                gid = gid,
                x = x,
                y = y,
                length = 1,
            }
        else
            -- Continue section
            section.length = section.length + 1
        end
    end

    sections[#sections + 1] = section
end

-- Find the most common tile (used as background)
local tile_counts = {}
for i,section in ipairs(sections) do
    tile_counts[section.gid] = (tile_counts[section.gid] or 0) + 1
end

local common_tile,highest = next(tile_counts)
for gid,count in pairs(tile_counts) do
    if count > highest then
        highest = count
        common_tile = gid
    end
end


local function crop_tile(gid)
    if gid == 0 then
        return
    end

    local id = gid - 1
    local columns = math.floor(tileset.imagewidth / tileset.tilewidth)

    local x = (id % columns) * tileset.tilewidth
    local y = math.floor(id / columns) * tileset.tileheight

    os.execute('convert ' .. tileset.image .. ' -crop ' ..
               tileset.tileheight .. 'x' .. tileset.tilewidth ..
               '+' .. x .. '+' .. y .. ' ../img/tile' .. gid .. '.png')
end


local lines = {}
local p = print

p('.t {')
p('    width: ' .. map.tilewidth .. 'px;')
p('    height: ' .. map.tileheight .. 'px;')
p('    position: absolute;')
p('}')
local used_tiles = {}
for gid,count in pairs(tile_counts) do
    used_tiles[#used_tiles + 1] = gid
end
table.sort(used_tiles, function(a, b) return a < b end)
for i,gid in ipairs(used_tiles) do
    if gid ~= common_tile then
        p('.t' .. gid .. ' { background: url(/img/tile' .. gid .. '.png); }')
    end
    crop_tile(gid)
end
p()

p('<div style="position: relative; ' ..
              'width: ' .. (map.width * map.tilewidth) .. 'px; ' ..
              'height: ' .. (map.height * map.tileheight) .. 'px; ' ..
              'background: url(/img/tile' .. common_tile .. '.png)">')

for i,section in ipairs(sections) do
    -- Don't print anything when it's the most common tile
    if section.gid ~= common_tile then
        local s = '  <div class="t t' .. section.gid .. '" style="' ..
                  'left: ' .. ((section.x - 1) * map.tilewidth) .. 'px; ' ..
                  'top: ' .. ((section.y - 1) * map.tileheight) .. 'px;'

        if section.length > 1 then
            s = s .. ' width: ' .. (section.length * map.tilewidth) .. 'px;'
        end

        p(s .. '"></div>')
    end
end

local objects = map.layers[2].objects
table.sort(objects, function(a, b) return a.y < b.y end)
for i,object in ipairs(objects) do
    p('  <div class="tree" style="left: ' .. object.x .. 'px; ' ..
                                 'top: ' .. (object.y - 160) .. 'px;"></div>')
end

print '</div>'
