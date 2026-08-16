using Godot;

public partial class Graveyard : Node2D
{
	[Export] public AudioStream MapMusic;
	private bool _readGrave;
	private bool _readNpc;
	private bool _disabled;

	public override void _Ready()
	{
		GetNode<MusicManager>("/root/MusicManager").PlayTrack(MapMusic);
	}

	
	private void OnGraveBodyEntered(Node2D body)
	{
		if (_readGrave) return;
		GetNode<Area2D>("Grave").GetNode<TextMenu>("GraveMenu").Init();
		_readGrave = true;
	}

	private void OnNpcBodyEntered(Node2D body)
	{
		if (_readNpc) return;
		GetNode<Area2D>("NPC/NPCArea").GetNode<TextMenu>("NPCMenu").Init();
		_readNpc = true;
	}

	private void DeleteDetectionArea()
	{
		if (_disabled) return;
		var detection1 = GetNode<Area2D>("Grave");
		var detection2 = GetNode<Area2D>("NPC/NPCArea");
		detection1.QueueFree();
		detection2.QueueFree();
		_disabled = true;
	}
}
