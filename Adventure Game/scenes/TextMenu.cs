using Godot;
using Godot.Collections;

public partial class TextMenu 
{
	[Signal]  public delegate void MessageReadEventHandler();
	
	[Export] public Array<string> Messages;
	private int _numberOfMessages;
	private int _currentMessage;
	private bool _canNext;
	private Array<string> _messages;
	public bool IsFinished;
}
public partial class TextMenu : CanvasLayer
{

	public override void _Ready()
	{
		_messages = Messages;
		_numberOfMessages = _messages.Count;
	}

	public override void _Process(double delta)
	{
		var player = GetTree().GetFirstNodeInGroup("Player") as Player;
		if (Visible)
		{
			if (player != null && player.CurrentState != Player.PlayerState.Battling) player.ChangeState(Player.PlayerState.InMenu);
		}

		if (!_canNext) return;
		
		if (Input.IsActionPressed("back") && _numberOfMessages <= 0)
		{
			EmitSignal(SignalName.MessageRead);
			IsFinished = true;
			_canNext = false;
			Visible = false;
			_currentMessage = 0;
			_numberOfMessages =  _messages.Count;
			if (player != null && player.CurrentState != Player.PlayerState.Battling) player.ChangeState(Player.PlayerState.Idle);
		}
		else if (Input.IsActionPressed("back") && _numberOfMessages > 0)
		{
			_canNext = false;
			SendMessages();
		}
	}

	public void Init()
	{
		SendMessages();
	}
	
	
	private void SendMessages()
	{
		if (_currentMessage >= _messages.Count)
		{
			Visible = false;
			return;
		}
		
		var messagePanel = GetNode<Label>("MarginContainer/MessageContainer/EventContainer/Message");
		messagePanel.Text = "";
		messagePanel.VisibleRatio = 0;
		
		Visible = true;
		messagePanel.Text = _messages[_currentMessage];
		
		var tween = CreateTween();
		tween.TweenProperty(messagePanel, "visible_ratio", 1.0,messagePanel.Text.Length * 0.03);
		tween.Finished += TweenOnFinished;
		_currentMessage++;
		_numberOfMessages--;
	}

	private void TweenOnFinished()
	{
		_canNext = true;
	}

	public void ChangeMessages(Array<string> newMessages)
	{
		_messages = newMessages;
		_numberOfMessages = newMessages.Count;
		_currentMessage = 0;
	}
}
