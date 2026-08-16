using Godot;

public partial class Finalbattle : Node2D
{
	[Export] public AudioStream MapMusic;
	
	public override void _Ready()
	{
		GetNode<MusicManager>("/root/MusicManager").PlayTrack(MapMusic);
	}

	private void OnExitAreaBodyEntered(Node2D body)
	{
		if (body is not Player) return;
		GetTree().Paused = true;
	}

	
}
