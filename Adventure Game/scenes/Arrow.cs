using Godot;


public partial class Arrow : Area2D
{
	
	[Signal] public delegate void HitEventHandler();
	private void OnBodyEntered(Node2D body)
	{
		if (body is Enemy) EmitSignal(SignalName.Hit);
	}
}
