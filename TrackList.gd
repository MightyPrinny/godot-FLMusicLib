extends ItemList

# Declare member variables here. Examples:
# var a = 2
# var b = "text"
# Called when the node enters the scene tree for the first time.
func _ready():
	var dir = Directory.new();
	dir.open("res://Music");
	dir.list_dir_begin(true,false);
	var current = dir.get_next();
	while current != null && current != "":
		add_item(current);
		
		current = dir.get_next();
	pass # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
