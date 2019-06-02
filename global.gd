extends Node

# Declare member variables here. Examples:
# var a = 2
# var b = "text"

var music_player = FLMusicLib.new();

# Create music player
func _ready():
	add_child(music_player);
	music_player.set_gme_buffer_size(2048*5);#optional
