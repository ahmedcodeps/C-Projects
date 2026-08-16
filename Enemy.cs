using Godot;

public partial class Enemy
{

	[Export] public AnimatedSprite2D Sprite { get; set; }
	[Export] public float Health { get; set; }
	[Export] public int Damage { get; set; }
	[Export] public string Name { get; set; }

	public enum EnemyState
	{
		Idle,
		Wandering,
		Following,
		Battling,
		Retreating,
	}
	

	private EnemyState _currentState;
	private Vector2 _moveDirection = Vector2.Zero;
	private const float Speed = 25.0f;
	private double _wanderTimer = 3.0;
	private double _wanderInterval = 1.0;
	private const int FollowRadius = 100;
	private float _health;
	private float _maxHealth;
	private bool _isHurting;
	private bool _isStop;

}
public partial class Enemy : CharacterBody2D
{
	public override void _Ready()
	{
		var sprite = GetNode<AnimatedSprite2D>("Sprite");
		sprite.SpriteFrames = Sprite.SpriteFrames;
		
		var collisionArea = GetNode<Area2D>("CollisionArea");
		collisionArea.BodyEntered += OnCollisionAreaBodyEntered;

		_health = Health;
		_maxHealth = Health;
		RandomizeDirAndWander();
	}

	public override void _Process(double delta)
	{
		var player = GetTree().GetFirstNodeInGroup("Player") as Player;
		
		if (player == null) return;
		
		if (_wanderTimer > 0 && _currentState == EnemyState.Wandering)
			_wanderTimer -= delta;
		else if (_wanderTimer <= 0 && _currentState != EnemyState.Battling && _currentState != EnemyState.Retreating)
			RandomizeDirAndWander();

		if (_wanderInterval > 0 && _currentState == EnemyState.Idle)
			_wanderInterval -= delta;
		else if (_wanderInterval <= 0 && _currentState != EnemyState.Battling && _currentState != EnemyState.Retreating)
			ChangeState(EnemyState.Wandering);

		if (player.GlobalPosition.DistanceTo(GlobalPosition) <= FollowRadius && _currentState != EnemyState.Battling && _currentState != EnemyState.Retreating)
			ChangeState(EnemyState.Following);
		else if (_currentState == EnemyState.Following &&
		         player.GlobalPosition.DistanceTo(GlobalPosition) >= FollowRadius && _currentState != EnemyState.Battling)
			RandomizeDirAndWander();

		if (_currentState == EnemyState.Retreating && player.GlobalPosition.DistanceTo(GlobalPosition) > FollowRadius)
			RandomizeDirAndWander();
		
	}


	public override void _PhysicsProcess(double delta)
	{
		var sprite = GetNode<AnimatedSprite2D>("Sprite");
		var player = GetTree().GetFirstNodeInGroup("Player") as Player;
		
		if (player == null) return;
		
		switch (_currentState)
		{
			case EnemyState.Idle:
				sprite.Play("idle");
				break;
			case EnemyState.Wandering:
				Velocity = _moveDirection * Speed;
				MoveAndSlide();
				sprite.Play("walk");
				sprite.FlipH = (_moveDirection.X < 0);
				break;
			case EnemyState.Following:
				Velocity = (player.GlobalPosition - GlobalPosition).Normalized() * Speed;
				MoveAndSlide();
				sprite.Play("walk");
				sprite.FlipH = (player.GlobalPosition.X < GlobalPosition.X);
				break;
			case EnemyState.Battling:
				sprite.FlipH = (player.GlobalPosition.X < GlobalPosition.X);
				if (!_isStop)
					sprite.Play(_isHurting ? "hurt" : "idle");
				break;
			case EnemyState.Retreating:
				var oppositeDir = -(player.GlobalPosition - GlobalPosition).Normalized();
				Velocity = oppositeDir * Speed;
				MoveAndSlide();
				sprite.Play("walk");
				sprite.FlipH = (oppositeDir.X < 0);
				break;
		}
	}

	private void RandomizeDirAndWander()
	{
		_wanderInterval = 1.0;
		ChangeState(EnemyState.Idle);
		
		_moveDirection = new Vector2(GD.RandRange(-1, 1), GD.RandRange(-1, 1)).Normalized();
		_wanderTimer = GD.RandRange(1, 3);
	}
	
	public void ChangeState(EnemyState newState)
	{
		_currentState = newState;
		if (newState == EnemyState.Idle)
			_isStop = false;
	}

	private void OnCollisionAreaBodyEntered(Node2D body)
	{
		var player = body as Player;
		
		if (player != null && player.CurrentState != Player.PlayerState.Battling && _currentState != EnemyState.Battling && player.CurrentState != Player.PlayerState.InMenu)
		{
			var battleManager = GetNode<BattleManager>("/root/BattleManager");
			ChangeState(EnemyState.Battling);
			player.ChangeState(Player.PlayerState.Battling);
			battleManager.StartBattle(player, this);
		}
		else if (player != null && player.CurrentState == Player.PlayerState.InMenu)
		{
			ChangeState(EnemyState.Retreating);
		}
	}

	private void TakeDamage(float damage)
	{
		_health -= damage;
	}

	public float GetHealth(bool max = false)
	{
		return max ? _maxHealth : _health;
	}

	public void ChangeHealth(float amount)
	{
		if (_health + amount >= _maxHealth)
		{
			_health += amount;
			_maxHealth = _health;
		}
		else
		{
			_health += amount;
		}
	}
	

	public async void Hurt(int damage)
	{
		var sprite = GetNode<AnimatedSprite2D>("Sprite");
		TakeDamage(damage);
		_isHurting = true;
		
		await ToSignal(sprite, AnimationMixer.SignalName.AnimationFinished);

		_isHurting = false;

	}

	public void StopProcess(bool stop)
	{
		_isStop = stop;
	}


}
