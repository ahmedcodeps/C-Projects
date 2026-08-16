using Godot;

public partial class Npc : CharacterBody2D
{
	[Export] public string NpcName { get; set; }
	[Export] public DialogueData Dialogue { get; set; }
	[Export] public bool ShowAllQuestionsBegin { get; set; }
	[Export] public AnimatedSprite2D Sprite { get; set; }
	

	private string _name;
	private DialogueData _dialogue;
	private const float CharReadRate = 0.05f;
	private bool _inRange;

	public enum NpcState
	{
		Idle,
		Talking,
		Walking
	}
	
	private Vector2 _direction = Vector2.Zero;
	private Vector2 _currentDirection = Vector2.Right;
	private NpcState _currentState = NpcState.Idle;
	private double _walkTimer = 5.0;
	private double _idleTimer = 5.0;
	private const float Speed = 20;

	private bool _canExit;

}
public partial class Npc
{
	public override void _Ready()
	{
		var area = GetNode<Area2D>("Area2D");
		area.BodyEntered += OnBodyEntered;
		area.BodyExited += OnBodyExited;
		
		var textbox = GetNode<CanvasLayer>("TextBox");
		var questions = textbox.GetNode<Panel>("Questions");
		_name = NpcName;
		_dialogue = Dialogue;

		textbox.GetNode<Control>("Name").GetNode<Label>("Label").Text = _name;
		GetNode<AnimatedSprite2D>("Sprite2D").SpriteFrames = Sprite.SpriteFrames;

		var children = questions.GetChildren();
		int count = Mathf.Min(children.Count, Dialogue.Entries.Count);
		
		
		for (int i = 0; i < count; i++)
		{
			var question = questions.GetChildren()[i];
			if (question is Button questionButton)
			{
				if (Dialogue.Entries[i] == null) break;
				questionButton.Text = Dialogue.Entries[i].Question;
			}
		}

		if (ShowAllQuestionsBegin)
		{
			foreach (var question in questions.GetChildren())
				if (question is Button questionButton && questionButton.Text != "default")
					questionButton.Visible = true;
		}
	}

	public override void _PhysicsProcess(double delta)
	{
		var sprite = GetNode<AnimatedSprite2D>("Sprite2D");
		
		switch (_currentState)
		{
			case NpcState.Idle or NpcState.Talking:
				sprite.Play("idle");
				sprite.FlipH = (_currentDirection.X < 0);
				break;
			case NpcState.Walking:
				Velocity = _currentDirection * Speed;
				MoveAndSlide(); 
				sprite.Play("walk");
				sprite.FlipH = (_currentDirection.X < 0);
				break;
		}
	}

	public override void _Process(double delta)
	{

		if (Dialogue.Entries.Count <= 0) return;
		
		if (Input.IsActionJustPressed("interact") && _inRange && _dialogue.Entries.Count > 0)
		{
			var player = GetParent().GetParent().GetParent().GetNode<Player>("Entities/Player");
			var textbox = GetNode<CanvasLayer>("TextBox");
			
			_currentDirection = (player.GlobalPosition - GlobalPosition).Normalized();
			
			textbox.Visible = !textbox.Visible;
			if (textbox.Visible)
			{
				_currentState = NpcState.Talking;
				player.ChangeState(Player.PlayerState.InMenu);
			}
			else
			{
				_currentState = NpcState.Idle;
				player.ChangeState(Player.PlayerState.Idle);
			}
		}
		
		var sprite = GetNode<AnimatedSprite2D>("Sprite2D");
		if (_currentState == NpcState.Idle)
		{
			if (_idleTimer > 0)
				_idleTimer -= delta;
			else
			{
				sprite.FlipH = (_currentDirection == Vector2.Left);
				ChangeState(NpcState.Walking);
				_idleTimer = 5.0;
			}
		}
		if (_currentState == NpcState.Walking)
		{
			if (_walkTimer > 0)
				_walkTimer -= delta;
			else
			{
				ChangeState(NpcState.Idle);
				_currentDirection = -_currentDirection;
				_walkTimer = 5.0;
			}
		}

		if (_canExit)
		{
			if (Input.IsActionPressed("back"))
			{
				_canExit = false;
				var textbox = GetNode<CanvasLayer>("TextBox");
				var questions = textbox.GetNode<Panel>("Questions");
				var answerContainer = textbox.GetNode<HBoxContainer>("MarginContainer/AnswerContainer/Answers");
				var answer = answerContainer.GetNode<Label>("Answer");
				questions.Visible = true;
				answerContainer.Visible = false;
				answer.VisibleRatio = 0.0f;
				answer.Text = "";
			}
		}
	}

	public void ChangeState(NpcState newState)
	{
		_currentState = newState;
	}

	private void OnBodyEntered(Node2D body)
	{
		if (body is Player) _inRange = true;
	}

	private void OnBodyExited(Node2D body)
	{
		if (body is Player)
		{
			var textbox = GetNode<CanvasLayer>("TextBox");
			textbox.Visible = false;
			_inRange = false;
			_currentState = NpcState.Idle;
		}
	}

	private void OnQ1Pressed() { SetUpAnswer(0); }
	private void OnQ2Pressed() { SetUpAnswer(1); }
	private void OnQ3Pressed() { SetUpAnswer(2); }
	private void OnQ4Pressed() { SetUpAnswer(3); }
	private void OnQ5Pressed() { SetUpAnswer(4); }
	
	private void TweenOnFinished()
	{
		_canExit = true;
	}

	private void SetUpAnswer(int index)
	{
		var textbox = GetNode<CanvasLayer>("TextBox");
		var questions = textbox.GetNode<Panel>("Questions");
		var answerContainer = textbox.GetNode<HBoxContainer>("MarginContainer/AnswerContainer/Answers");
		var answer = answerContainer.GetNode<Label>("Answer");
		questions.Visible = false;
		answerContainer.Visible = true;
		var tween = CreateTween();
		answer.Text = Dialogue.Entries[index].Answer;
		tween.TweenProperty(answer, "visible_ratio", 1.0,answer.Text.Length * CharReadRate);
		tween.Finished += TweenOnFinished;

		if (!ShowAllQuestionsBegin && index + 1 < questions.GetChildren().Count)
		{
			var question = questions.GetChildren()[index + 1] as Button;
			if (question == null)
			{
				GD.PrintErr("[ERROR] : NPC : Question not found");
			}
			else
			{
				question.Visible = true;
			}
		}
	}

	public void ChangeDirection(Vector2 newDirection)
	{
		_currentDirection = newDirection;
	}
	
}
