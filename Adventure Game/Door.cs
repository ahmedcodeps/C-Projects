using Godot;

public partial class Door : Area2D
{
	[Export] private int _doorLeadsTo;
	
}
public partial class Door
{
	
	public override void _Ready()
	{
		BodyEntered += OnBodyEntered;
	}
	
	private void OnBodyEntered(Node2D body)
	{
		if (body is not Player) return;
		
		var game = GetParent().GetParent().GetParent() as Game;
		if (game == null)
			GD.PrintErr("[ERROR] : DOOR : Game is null.");
		else
			game.SwitchMap(_doorLeadsTo);
	}
	
}
