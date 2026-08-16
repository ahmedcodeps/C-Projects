using Godot;


public partial class Lagoon : Node2D
{
	[Export] public AudioStream MapMusic;
	private Player _player;
	
	public override void _Ready()
	{
		GetNode<MusicManager>("/root/MusicManager").PlayTrack(MapMusic);
	}

	
	private void OnMessageBodyEntered(Node2D body)
	{
		var messages = GetNode<Node2D>("Messages");
		var message = messages.GetNode<TextMenu>("Message/MessageMenu");
		message.MessageRead += OnMessage1Read;
		message.Init();
	}
	
	private void OnMessage2BodyEntered(Node2D body)
	{
		var messages = GetNode<Node2D>("Messages");
		var message2 = messages.GetNode<TextMenu>("Message2/MessageMenu");
		message2.MessageRead += OnMessage2Read;
		message2.Init();
	}

	private void OnTheDoctorBodyEntered(Node2D body)
	{
		var message = GetNode<Area2D>("TheDoctor").GetNode<TextMenu>("DoctorMessage");
		message.MessageRead += OnDoctorMessageRead;
		message.Init();
		_player = body as Player;
	}

	private void OnMessage1Read()
	{
		var messages = GetNode<Node2D>("Messages");
		var message = messages.GetNode<TextMenu>("Message/MessageMenu");
		message.MessageRead -= OnMessage1Read;
		message.QueueFree();
	}

	private void OnMessage2Read()
	{
		var messages = GetNode<Node2D>("Messages");
		var message2 = messages.GetNode<TextMenu>("Message2/MessageMenu");
		message2.MessageRead -= OnMessage2Read;
		message2.QueueFree();
	}

	private void OnDoctorMessageRead()
	{
		var message = GetNode<Area2D>("TheDoctor").GetNode<TextMenu>("DoctorMessage");
		message.MessageRead -= OnDoctorMessageRead;
		message.QueueFree();

		GetNode<Item>("Items/Key").GlobalPosition = _player.GlobalPosition;
		GetNode<Door>("Door").Visible = true;
		GetNode<PointLight2D>("Door/DoorLight").Visible = true;
	}
	

	
}
