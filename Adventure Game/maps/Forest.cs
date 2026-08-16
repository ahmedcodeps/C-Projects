using Godot;

public partial class Forest : Node2D
{
	[Export] public AudioStream MapMusic;
	private bool _readNpc;
	private bool _disabled = true;
	
	public override void _Ready()
	{
		GetNode<MusicManager>("/root/MusicManager").PlayTrack(MapMusic);
	}

	
	private void OnNpcBodyEntered(Node2D body)
	{
		if (_readNpc) return;
		var npc = GetNode<Npc>("NPC");
		GetNode<Area2D>("NPC/NPCArea").GetNode<TextMenu>("NPCMenu").Init();
		npc.ChangeState(Npc.NpcState.Talking);
		_readNpc = true;
		_disabled = false;
	}

	private void DeleteDetectionArea()
	{
		if (_disabled) return;
		var detection1 =  GetNode<Area2D>("NPC/NPCArea");
		detection1.QueueFree();
		_disabled = true;
	}
}
