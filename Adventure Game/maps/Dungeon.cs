using Godot;

public partial class Dungeon : Node2D
{
	[Export] public AudioStream MapMusic;
	private bool _readMagician;
	public override void _Ready()
	{
		GetNode<MusicManager>("/root/MusicManager").PlayTrack(MapMusic);
	}
	
	private void OnMagicianBodyEntered(Node2D body)
	{
		if (_readMagician) return;
		if (body is not Player) return;
		GetNode<TextMenu>("NPC/Magician/MagicMenu").Init();
		
		var npc = GetNode<Npc>("NPC");
		var newDirection = (body.GlobalPosition - npc.GlobalPosition).Normalized();
		
		npc.ChangeDirection(newDirection);
		_readMagician = true;
	}

	private void DeleteDetectionArea()
	{
		var magic = GetNode<Area2D>("NPC/Magician");
		magic.QueueFree();
	}

}
