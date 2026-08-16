using Godot;

public partial class OldTown : Node2D
{
    [Export] public AudioStream MapMusic;
    public override void _Ready()
    {
        GetNode<MusicManager>("/root/MusicManager").PlayTrack(MapMusic);
    }
    
    
}
