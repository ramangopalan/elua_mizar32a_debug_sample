
local PIN_BUTTON  = 2
local PIN_AUTOPLAY = 1
local PIN_READWRITE = 10
local PIN_CONTRAST = 12

local SPRITE_RUN1 = 1
local SPRITE_RUN2 = 2
local SPRITE_JUMP = 3
local SPRITE_JUMP_UPPER = '.'         -- Use the '.' character for the head
local SPRITE_JUMP_LOWER = 4
local SPRITE_TERRAIN_EMPTY = ' '      -- User the ' ' character
local SPRITE_TERRAIN_SOLID = 5
local SPRITE_TERRAIN_SOLID_RIGHT = 6
local SPRITE_TERRAIN_SOLID_LEFT = 7

local HERO_HORIZONTAL_POSITION = 1    -- Horizontal position of hero on screen

local TERRAIN_WIDTH = 16
local TERRAIN_EMPTY = 0
local TERRAIN_LOWER_BLOCK = 1
local TERRAIN_UPPER_BLOCK = 2

local HERO_POSITION_OFF = 0          -- Hero is invisible
local HERO_POSITION_RUN_LOWER_1 = 1  -- Hero is running on lower row (pose 1)
local HERO_POSITION_RUN_LOWER_2 = 2  --                              (pose 2)

local HERO_POSITION_JUMP_1 = 3       -- Starting a jump
local HERO_POSITION_JUMP_2 = 4       -- Half-way up
local HERO_POSITION_JUMP_3 = 5       -- Jump is on upper row
local HERO_POSITION_JUMP_4 = 6       -- Jump is on upper row
local HERO_POSITION_JUMP_5 = 7       -- Jump is on upper row
local HERO_POSITION_JUMP_6 = 8       -- Jump is on upper row
local HERO_POSITION_JUMP_7 = 9       -- Half-way down
local HERO_POSITION_JUMP_8 = 10      -- About to land

local HERO_POSITION_RUN_UPPER_1 = 11 -- Hero is running on upper row (pose 1)
local HERO_POSITION_RUN_UPPER_2 = 12 --                              (pose 2)

terrainUpper = {} -- [TERRAIN_WIDTH + 1];
terrainLower = {} -- [TERRAIN_WIDTH + 1];
buttonPushed = false;

heroPos = HERO_POSITION_RUN_LOWER_1
newTerrainType = TERRAIN_EMPTY
newTerrainDuration = 1
playing = false
blink = false
distance = 0

-- Graphics.

function initializeGraphics()
  -- Skip using character 0, this allows lcd.print() to be used to
  -- quickly draw multiple characters

  -- Run Position 1
  run_position_1 = {12, 12, 0, 14, 28, 12, 26, 19}
  avr32.lcd.definechar(1, run_position_1)
  
  -- Run Position 2
  run_position_2 = {12, 12, 0, 12, 12, 12, 12, 14}
  avr32.lcd.definechar(2, run_position_2)
  
  -- Jump
  jump = {12, 12, 0, 30, 13, 31, 16, 0}
  avr32.lcd.definechar(3, jump)
  
  -- Jump lower
  jump_lower = {30, 13, 31, 16, 0, 0, 0, 0}
  avr32.lcd.definechar(4, jump_lower)
  
  -- Ground
  ground = {31, 31, 31, 31, 31, 31, 31, 31}
  avr32.lcd.definechar(5, ground)
  
  -- Ground Right
  ground_right = {3, 3, 3, 3, 3, 3, 3, 3}
  avr32.lcd.definechar(6, ground_right)
  
  -- Ground Left.
  ground_left = {24, 24, 24, 24, 24, 24, 24, 24}
  avr32.lcd.definechar(7, ground_left)
  
  for i = 0, TERRAIN_WIDTH do
    terrainUpper[i] = SPRITE_TERRAIN_EMPTY
    terrainLower[i] = SPRITE_TERRAIN_EMPTY
  end
end

-- Slide the terrain to the left in half-character increments
function advanceTerrain(terrain, newTerrain)
  for i = 0, TERRAIN_WIDTH do
    current = terrain[i]
    next = (function() if i == TERRAIN_WIDTH-1 then return newTerrain else return terrain[i+1] end end)()
    if     current == SPRITE_TERRAIN_EMPTY then
      terrain[i] = (function() if next == SPRITE_TERRAIN_SOLID then return SPRITE_TERRAIN_SOLID_RIGHT else return SPRITE_TERRAIN_EMPTY end end)()
    elseif current == SPRITE_TERRAIN_SOLID then
      terrain[i] = (function() if next == SPRITE_TERRAIN_EMPTY then return SPRITE_TERRAIN_SOLID_LEFT else return SPRITE_TERRAIN_SOLID end end)()
    elseif current == SPRITE_TERRAIN_SOLID_RIGHT then
      terrain[i] = SPRITE_TERRAIN_SOLID
    elseif current == SPRITE_TERRAIN_SOLID_LEFT then
      terrain[i] = SPRITE_TERRAIN_EMPTY
    end
  end
end

function drawHero(position, terrainUpper, terrainLower, score)
  collide = false
  upperSave = terrainUpper[HERO_HORIZONTAL_POSITION]
  lowerSave = terrainLower[HERO_HORIZONTAL_POSITION]
  local upper = 0
  local lower = 0
  if     position == HERO_POSITION_OFF then
    upper = SPRITE_TERRAIN_EMPTY
	lower = SPRITE_TERRAIN_EMPTY
  elseif position == HERO_POSITION_RUN_LOWER_1 then
    upper = SPRITE_TERRAIN_EMPTY
	lower = SPRITE_RUN1
  elseif position == HERO_POSITION_RUN_LOWER_2 then
    upper = SPRITE_TERRAIN_EMPTY
	lower = SPRITE_RUN2
  elseif position == HERO_POSITION_JUMP_1 then
    upper = SPRITE_TERRAIN_EMPTY
	lower = SPRITE_JUMP
  elseif position == HERO_POSITION_JUMP_8 then
    upper = SPRITE_TERRAIN_EMPTY
	lower = SPRITE_JUMP
  elseif position == HERO_POSITION_JUMP_2 then
    upper = SPRITE_JUMP_UPPER
	lower = SPRITE_JUMP_LOWER
  elseif position == HERO_POSITION_JUMP_7 then
    upper = SPRITE_JUMP_UPPER
	lower = SPRITE_JUMP_LOWER
  elseif position == HERO_POSITION_JUMP_3 then
    upper = SPRITE_JUMP
	lower = SPRITE_TERRAIN_EMPTY
  elseif position == HERO_POSITION_JUMP_4 then
    upper = SPRITE_JUMP
	lower = SPRITE_TERRAIN_EMPTY
  elseif position == HERO_POSITION_JUMP_5 then
    upper = SPRITE_JUMP
	lower = SPRITE_TERRAIN_EMPTY
  elseif position == HERO_POSITION_JUMP_6 then
    upper = SPRITE_JUMP
	lower = SPRITE_TERRAIN_EMPTY
  elseif position == HERO_POSITION_RUN_UPPER_1 then
    upper = SPRITE_RUN1
	lower = SPRITE_TERRAIN_EMPTY
  elseif position == HERO_POSITION_RUN_UPPER_2 then
    upper = SPRITE_RUN2
	lower = SPRITE_TERRAIN_EMPTY
  end

  if upper ~= ' ' then
    terrainUpper[HERO_HORIZONTAL_POSITION] = upper
	collide = (function() if upperSave == SPRITE_TERRAIN_EMPTY then return false else return true end end)()
  end

  if lower ~= ' ' then
    terrainLower[HERO_HORIZONTAL_POSITION] = lower;
	collide = collide or (function() if lowerSave == SPRITE_TERRAIN_EMPTY then return false else return true end end)()
    -- Raman: Check above: collide |= ( == ) ? false : true;
  end
  
  digits = (function() if score > 9999 then return 5 elseif score > 999 then return 4 elseif score > 99 then return 3 elseif score > 9 then return 2 else return 1 end end)()
  
  -- Draw the scene
  terrainUpper[TERRAIN_WIDTH] = ''
  terrainLower[TERRAIN_WIDTH] = ''
  temp = terrainUpper[16-digits]
  terrainUpper[16-digits] = ''
  avr32.lcd.goto(1,1)
  for k, v in ipairs(terrainUpper) do
    avr32.lcd.print(v)
  end

  terrainUpper[16-digits] = temp
  avr32.lcd.goto(2,1);
  for k, v in ipairs(terrainLower) do
    avr32.lcd.print(v)
  end
  
  avr32.lcd.goto(1, 16 - digits)
  avr32.lcd.print(tostring(score))

  terrainUpper[HERO_HORIZONTAL_POSITION] = upperSave
  terrainLower[HERO_HORIZONTAL_POSITION] = lowerSave
  return collide
end

-- Handle the button push as an interrupt
function checkButtonPush()
  pressed = avr32.lcd.buttons()
  if pressed:find("R") then
    buttonPushed = true
  else
    buttonPushed = false
  end
end

function setup()
  --[[
  pinMode(PIN_READWRITE, OUTPUT);
  digitalWrite(PIN_READWRITE, LOW);
  pinMode(PIN_CONTRAST, OUTPUT);
  digitalWrite(PIN_CONTRAST, LOW);
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_BUTTON, HIGH);
  pinMode(PIN_AUTOPLAY, OUTPUT);
  digitalWrite(PIN_AUTOPLAY, HIGH);
  --]]
  
  -- Digital pin 2 maps to interrupt 0
  -- Well, we refrain from interrupts on the Mizar32.
  -- attachInterrupt(0/*PIN_BUTTON*/, buttonPush, FALLING);
  
  initializeGraphics();
  
  --lcd.begin(16, 2);
end

function loop()
  --heroPos = HERO_POSITION_RUN_LOWER_1
  --newTerrainType = TERRAIN_EMPTY
  --newTerrainDuration = 1
  --playing = false
  --blink = false
  --distance = 0

  -- Keep checking if the button is pressed.
  checkButtonPush()
  
  if (playing == false) then
    drawHero((function() if blink == true then return HERO_POSITION_OFF else return heroPos end end)(), terrainUpper, terrainLower, bit.rshift(distance, 3))
    if blink == true then
      avr32.lcd.goto(1,1);
      avr32.lcd.print("Press Start");
    end
    tmr.delay(0, 2500000);
    blink = not blink;
    if buttonPushed == true then
      initializeGraphics();
      heroPos = HERO_POSITION_RUN_LOWER_1;
      playing = true;
      buttonPushed = false;
      distance = 0;
    end
    do return end
  end

  -- Shift the terrain to the left
  advanceTerrain(terrainLower, (function() if newTerrainType == TERRAIN_LOWER_BLOCK then return SPRITE_TERRAIN_SOLID else return SPRITE_TERRAIN_EMPTY end end)());
  advanceTerrain(terrainUpper, (function() if newTerrainType == TERRAIN_UPPER_BLOCK then return SPRITE_TERRAIN_SOLID else return SPRITE_TERRAIN_EMPTY end end)());
  
  -- Make new terrain to enter on the right
  newTerrainDuration = newTerrainDuration - 1
  if newTerrainDuration == 0 then
    if newTerrainType == TERRAIN_EMPTY then
      newTerrainType = (function() if math.random(0, 3) == 0 then return TERRAIN_UPPER_BLOCK else return TERRAIN_LOWER_BLOCK end end)()
      newTerrainDuration = 2 + math.random(0, 10);
    else
      newTerrainType = TERRAIN_EMPTY;
      newTerrainDuration = 10 + math.random(0, 10);
    end
  end
    
  if buttonPushed == true then
    if heroPos <= HERO_POSITION_RUN_LOWER_2 then heroPos = HERO_POSITION_JUMP_1 end
    buttonPushed = false;
  end

  if drawHero(heroPos, terrainUpper, terrainLower, bit.rshift(distance, 3)) == true then
    playing = false; -- The hero collided with something. Too bad.
  else
    if heroPos == HERO_POSITION_RUN_LOWER_2 or heroPos == HERO_POSITION_JUMP_8 then
      heroPos = HERO_POSITION_RUN_LOWER_1
    elseif ((heroPos >= HERO_POSITION_JUMP_3 and heroPos <= HERO_POSITION_JUMP_5) and terrainLower[HERO_HORIZONTAL_POSITION] ~= SPRITE_TERRAIN_EMPTY) then
      heroPos = HERO_POSITION_RUN_UPPER_1;
    elseif (heroPos >= HERO_POSITION_RUN_UPPER_1 and terrainLower[HERO_HORIZONTAL_POSITION] == SPRITE_TERRAIN_EMPTY) then
      heroPos = HERO_POSITION_JUMP_5;
    elseif (heroPos == HERO_POSITION_RUN_UPPER_2) then
      heroPos = HERO_POSITION_RUN_UPPER_1;
    else
      heroPos = heroPos + 1
    end
    distance = distance + 1
    
    -- TODO: Raman: digitalWrite(PIN_AUTOPLAY, terrainLower[HERO_HORIZONTAL_POSITION + 2] == SPRITE_TERRAIN_EMPTY ? HIGH : LOW);
  end
  tmr.delay(0, 100000);
end

setup()

-- Debug only: avr32.lcd.print(1, 2, 3, 4, 5, 6, 7)

while true do
  loop()
end
-- Hi Sergio. I am within a GNU Ed session. Editing
-- hero, the game. Typing is easy. I don't remember
-- all commands.
