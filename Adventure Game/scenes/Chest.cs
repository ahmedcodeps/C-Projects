using Godot;

public partial class Chest : Area2D
{
	[Export] public Item ChestItem;
	[Export] private bool _requiresKey;
	
	private bool _inOpenRadius;
	private bool _tookItem;
	private Player _player;

	public override void _Ready()
	{
		ChestItem.GetNode<CollisionShape2D>("CollisionShape2D").Disabled = true;
		ChestItem.Visible = false;
	}

	private bool CheckForKey()
	{
		if (_player == null) return false;
		foreach (var slot in _player.Slots)
		{
			if (slot.ItemRef == null) continue;
			
			if (slot.ItemRef.ItemName == "key")
			{
				_player.Eat(slot.ItemRef.ItemName, slot.ItemRef);
				return true;
			}
		}
		return false;
	}

	public override void _Process(double delta)
	{
		if (_inOpenRadius && _player != null && Input.IsActionJustPressed("interact") && !_tookItem)
		{
			if (!CheckForKey() && _requiresKey)
			{
				var textMenu = GetNode<TextMenu>("TextMenu");
				textMenu.Init();
				return;
			}
			
			_tookItem = true;
			var openAnim = GetNode<AnimatedSprite2D>("Open");
			openAnim.Play("open");
			ChestItem.Visible = true;
			ChestItem.GetNode<CollisionShape2D>("CollisionShape2D").Disabled = false;
			ChestItem.GlobalPosition = _player.GlobalPosition;
		}
	}

	private void OnBodyEntered(Node2D body)
	{
		if (body is Player player)
		{
			_inOpenRadius = true;
			_player = player;
		}
	}

	private void OnBodyExited(Node body)
	{
		if (body is Player) _inOpenRadius = false;
	}
	
}
