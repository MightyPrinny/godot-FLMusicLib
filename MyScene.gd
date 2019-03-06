extends Node2D

#Nodes
onready var itemList:ItemList = $ItemList;
onready var loadBtn = $Load;
onready var playPause = $Btns/Play;
onready var storeBtn = $Btns/Store;
onready var restoreBtn = $Btns/Restore;

onready var GMEPlayer = $"/root/FLMusic";

var loopStart = 0;
var loopEnd = 0;
var loop = false;
var trackNum = 0;
var vol = "1";

func _ready():
	loadBtn.connect("pressed",self,"_play_music");
	playPause.connect("pressed",self,"toggle_pause");
	storeBtn.connect("pressed",self,"store_music_state");
	restoreBtn.connect("pressed",self,"restore_music_state");
	GMEPlayer.connect("track_ended",self,"_track_ended");
	pass

func _play_music():
	if !itemList.is_anything_selected():
		return;
	var mus = "res://Music/"+itemList.get_item_text(itemList.get_selected_items()[0]);
	GMEPlayer.PlayMusic(mus,trackNum,loop,loopStart,loopEnd,0);
	set_volume(vol);
	
func  stop_music():
	GMEPlayer.StopMusic();
		
func set_loop_end(var n):
	loopEnd = n.to_int();
	
func set_loop_start(var n):
	loopStart = n.to_int();
	
func set_track_number(var n):
	trackNum = n.to_int();
	
func set_loop(var b):
	loop = b;		
func toggle_pause():
	GMEPlayer.TogglePause();
	pass
		
func store_music_state():
	GMEPlayer.StoreMusicState();
	pass
	
func restore_music_state():
	GMEPlayer.RestoreMusicState();
	pass
	
func _track_ended():
	print("end");
	
func set_volume(var t):
	vol = t;
	GMEPlayer.SetVolumeMultiplier(t.to_float());
