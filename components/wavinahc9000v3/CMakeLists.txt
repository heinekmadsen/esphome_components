esphome_component_register(
  COMPONENT_NAME wavin_ahc9000
  SRC wavin_ahc9000.cpp
  INCLUDE_DIRS .
  REQUIRES uart climate sensor switch
)
