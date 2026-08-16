using Godot;
using System;
using Godot.Collections;

public partial class Battle
{
	
	[Export] public AudioStream BattleMusic;
	[Export] public AudioStream BossMusic;
	private Player _player;
	private Enemy _enemy;

	private Control _inventoryClone;

	private Vector2 _playerStatsOffset = new Vector2(190, -250);
	private Vector2 _enemyStatsOffset = new Vector2(-5, -250);

	private enum Action
	{
		Attack,
		Eat,
		Defend,
		Nothing,
	}

	private enum EnemyAction
	{
		Attack,
		Defend,
		Special,
		Nothing,
		Dead,
	}

	private enum Turn
	{
		Enemy,
		Player,
	}
	
	private Dictionary<int, string> _anims = new Dictionary<int, string>()
	{
		{0, "attack1"},
		{1, "attack2"},
	};
	
	private Dictionary<string, string> _enemyDescriptions = new Dictionary<string, string>()
	{
		{"Slime", "You enter battle with a small, gelatinous creature named a slime. You have been warned about the healing capabilities of this creature."},
		{"Orc", "You enter battle with a violent green creature wielding an axe named an Orc. Apparently, these creatures can increase their own strength with rage."},
		{"Creeper", "You enter battle with an ominous hand creature named a creeper. These monsters can take items from your inventory."},
		{"Goblin", "You enter battle with an extremely strong and tall beast named a goblin. Getting too close risks getting poked by a spear."},
		{"BDemon", "You enter battle with a red demon named a Blood Demon. These beings can use blood from damage inflicted on them to increase their attack."},
		{"HDemon", "You have entered battle with a horned demon named the Horn Demon. These demons can use their speed to make multiple decisions in quick succession."},
		{"Boss", "You have entered battle with the strongest magician in the realm. He wields multiple abilities and is the most durable of fighters, but beating him means returning to your world."}
	};


	private Action _currentAction;
	private EnemyAction _currentEnemyAction;
	private Turn _currentTurn;
	private bool _isAttacking;
	private bool _enemyIsAttacking;
	private float _attackRange = 25.0f;
	private float _playerSpeed = 50f;

	
	private int _playerDamage;
	private Vector2 _initialPlayerPos;
	private Vector2 _initialEnemyPos;
	private bool _usedSpecialDefense;
	private bool _usedSpecialAttack;
	private int _turnsOfDefense;
	private int _turnsOfAttack;
	private float _originalEnemyHp;
	private int _maxFireUp = 4;

	private bool _playerAttacked;
	private bool _playerReturning;
	private bool _enemyAttacked;
	private bool _enemyReturning;

	private int _turnsOfShield = 3;
	private bool _shielding;
	private bool _fightingGoblin;

	private bool _isAttackingWithBow;

	private bool _usedPoison;
}
public partial class Battle : Node2D
{
	public void Init(Player player, Enemy enemy)
	{
		var playerSpawn = GetNode<Marker2D>("PlayerPoint").GlobalPosition;
		var enemySpawn = GetNode<Marker2D>("EnemyPoint").GlobalPosition;
		var battleCamera = GetNode<Camera2D>("BattleCamera");
		battleCamera.MakeCurrent();
		
		player.Owner = null;
		enemy.Owner = null;
		
		player.GetParent().CallDeferred("remove_child", player);
		enemy.GetParent().CallDeferred("remove_child", enemy);
		CallDeferred("add_child", player);
		CallDeferred("add_child", enemy);
		
		player.GlobalPosition = playerSpawn;
		enemy.GlobalPosition = enemySpawn;

		_player = player;
		_enemy = enemy;

		_originalEnemyHp = _enemy.GetHealth();
		
		_currentTurn = Turn.Player;

		if (_enemy.Name == "Goblin")
			_fightingGoblin = true;

		var music = GetNode<MusicManager>("/root/MusicManager");
		music.PlayTrack(_enemy.Name == "Boss" ? BossMusic : BattleMusic);
		
		
		SetUi();
	}

	public void LeaveBattle(bool playerDead = false)
	{
		
		var battleManager = GetNode<BattleManager>("/root/BattleManager");
    
		if (_currentEnemyAction == EnemyAction.Dead)
		{
			battleManager.EndBattle(this, _player);
			return;
		}
		
		battleManager.EndBattle(this, _player, _enemy, playerDead);
	}

	private void SetUi()
	{
		var battleUi = GetNode<CanvasLayer>("BattleUI");
		var playerStatsPanel = battleUi.GetNode<PanelContainer>("PlayerStatsPanel");
		var enemyStatsPanel = battleUi.GetNode<PanelContainer>("EnemyStatsPanel");
		var enemyNameLabel = enemyStatsPanel.GetNode<Label>("NameLabel");
		var enemyHealthLabel = enemyStatsPanel.GetNode<Label>("HealthLabel");
		var playerNameLabel = playerStatsPanel.GetNode<Label>("NameLabel");
		var playerHealthLabel = playerStatsPanel.GetNode<Label>("HealthLabel");
		var buttons = battleUi.GetNode<VBoxContainer>("ActionPanel/ActionsContainer");

		playerStatsPanel.GlobalPosition = _player.GlobalPosition + _playerStatsOffset;
		enemyStatsPanel.GlobalPosition = _player.GlobalPosition + _enemyStatsOffset;

		enemyNameLabel.Text = _enemy.Name;
		enemyHealthLabel.Text = _enemy.GetHealth() + "/" + _enemy.GetHealth(true);
		playerNameLabel.Text = _player.Name.ToString();
		playerHealthLabel.Text = _player.GetHealth() + "/" + _player.MaxHealth;
		
		var inventoryClone = _player.GetNode<Control>("Inventory").Duplicate() as Control;
		
		if (inventoryClone == null)
			GD.PrintErr("[ERROR] : Battle : The player lacks an inventory");
		else
		{
			AddChild(inventoryClone);
			inventoryClone.GlobalPosition = _player.GlobalPosition;
			_inventoryClone = inventoryClone;
		}
		

		foreach (var button in buttons.GetChildren())
		{
			if (button is Button btn)
				btn.FocusMode = Control.FocusModeEnum.None;
		}
		
		SetUpInventoryButtons();
		
		SendMessageForAction(_enemyDescriptions[_enemy.Name]);

	}

	public override void _Process(double delta)
	{
		var playerHealthLabel = GetNode<Label>("BattleUI/PlayerStatsPanel/HealthLabel");
		var enemyHealthLabel =  GetNode<Label>("BattleUI/EnemyStatsPanel/HealthLabel");
		
		playerHealthLabel.Text = _player.GetHealth() + "/" + _player.MaxHealth;
		enemyHealthLabel.Text = _enemy.GetHealth() + "/" + _enemy.GetHealth(true);
	}

	private double _range = 1200;
	private double _travelDistance;
	private bool _played;
	public override void _PhysicsProcess(double delta)
	{
		if (!IsInstanceValid(_enemy) || !IsInstanceValid(_player)) return;
		
		var anims = _player.GetNode<AnimatedSprite2D>("Anims");
		var enemyAnims = _enemy.GetNode<AnimatedSprite2D>("Sprite");
		if (_currentAction == Action.Attack && _isAttacking)
		{
			if (_player.GlobalPosition.DistanceTo(_enemy.GlobalPosition) >= _attackRange && !_playerReturning)
			{
				
				_player.Velocity = (_enemy.GlobalPosition - _player.GlobalPosition).Normalized() * _playerSpeed;
				_player.MoveAndSlide();
				anims.Play("walk");
			}
			else
			{
				if (!_playerAttacked)
				{
					anims.Play("idle");
					_playerAttacked = true;
					_player.Velocity = Vector2.Zero;
					RunAttackSequence();
				}
				else if(_playerReturning)
				{
					if (_player.GlobalPosition.DistanceTo(_initialPlayerPos) >= 2.0f)
					{
						_player.Velocity = _player.GlobalPosition.DirectionTo(_initialPlayerPos) * _playerSpeed;
						_player.MoveAndSlide();
						anims.Play("walk");
					}
					else
					{
						_player.Velocity = Vector2.Zero;
						_player.GlobalPosition = _initialPlayerPos;
						anims.Play("idle");

						_playerAttacked = false;
						_playerReturning = false;
						_isAttacking = false;
						PlayerTurn();
						_player.ToggleAttack();
						_currentAction = Action.Nothing;
					}
					
				}
			}
		}
		
		else if (_currentAction == Action.Attack && _isAttackingWithBow)
		{
			var arrow = _player.GetNode<Arrow>("Arrow");

			if (!_played)
			{
				arrow.Hit += OnArrowHit;
				_played = true;
				
				var anim = _player.GetNode<AnimatedSprite2D>("Anims");
				anim.Play("shoot");
				
				arrow.Visible = true;
			}
			
			var arrowDirection = (_enemy.GlobalPosition - arrow.GlobalPosition).Normalized();
			var arrowSpeed = 100.0 * delta;
			var arrowVelocity = arrowDirection * (float)arrowSpeed;
			
			arrow.Position += arrowVelocity;
			
			_travelDistance += arrowSpeed * delta;

			if (_travelDistance >= _range)
			{
				arrow.Visible = false;
				arrow.GlobalPosition = _player.GlobalPosition;
				_travelDistance = 0;
			}
		}

		if (_currentTurn != Turn.Enemy) return;

		if (_currentEnemyAction == EnemyAction.Attack && _enemyIsAttacking)
		{
			if (_enemy.GlobalPosition.DistanceTo(_player.GlobalPosition) >= _attackRange && !_enemyReturning)
			{
				_enemy.Velocity = (_player.GlobalPosition - _enemy.GlobalPosition).Normalized() * _playerSpeed;
				_enemy.MoveAndSlide();
				enemyAnims.Play("walk");
				enemyAnims.FlipH = false;
			}
			else
			{
				if (!_enemyAttacked)
				{
					enemyAnims.Play("idle");
					_enemyAttacked = true;
					_enemy.Velocity = Vector2.Zero;
					RunAttackSequence();
				}
				else if (_enemyReturning)
				{
					if (_enemy.GlobalPosition.DistanceTo(_initialEnemyPos) >= 2.0f)
					{
						_enemy.Velocity = _enemy.GlobalPosition.DirectionTo(_initialEnemyPos) *  _playerSpeed;
						_enemy.MoveAndSlide();
						enemyAnims.Play("walk");
					}
					else
					{
						_enemy.Velocity = Vector2.Zero;
						_enemy.GlobalPosition = _initialEnemyPos;
						enemyAnims.Play("idle");
					
						ToggleStopEnemyProcess();
						_enemyReturning = false;
						_enemyAttacked = false;
						_isAttacking = false;
						_currentEnemyAction = EnemyAction.Nothing;
						_enemyIsAttacking = false;
						if (_enemy.Name != "HDemon") _currentTurn = Turn.Player; 
						
						if (_usedPoison)
						{
							_enemy.Hurt(3);
							SendMessageForAction("The " + _enemy.Name + " was damaged by the poison!");
							HandleEnemyDeath();
						}

						if (_enemy.Name != "HDemon") return;
						
						var rand = GD.RandRange(0, 1);
						if (rand == 1)
							_currentTurn = Turn.Player;
						else
						{
							SendMessageForAction("The Horn Demon used its speed to make another move!");
							HDemonTurn();
						}
						
					}
				}
			}
		}
		
		
	}
	
	private void OnArrowHit()
	{
		var arrow = _player.GetNode<Arrow>("Arrow");
		arrow.Hit -= OnArrowHit;
		
		arrow.Visible = false;
		arrow.GlobalPosition = _player.GlobalPosition;
		_travelDistance = 0;
		
		CalculateAndAttack();
		
		_isAttackingWithBow = false;
		_played = false;
		_currentAction = Action.Nothing;
		
		if (_enemy.GetHealth() <= 0) return;
		
		_player.ToggleAttack();
		
		TurnEnd(2.0f);
	}

	private async void RunAttackSequence()
	{
		var anims = _player.GetNode<AnimatedSprite2D>("Anims");
		if (_currentTurn == Turn.Enemy) anims = _enemy.GetNode<AnimatedSprite2D>("Sprite");
		
		
		CalculateAndAttack();
		await ToSignal(anims, AnimationMixer.SignalName.AnimationFinished);

		if (_currentTurn == Turn.Enemy) _enemyReturning = true;
		else _playerReturning = true;
	}

	private void OnUseButtonPressed()
	{
		if (_currentTurn != Turn.Player || _isAttacking) return;
		_inventoryClone.Visible = !_inventoryClone.Visible;
	}

	private async void OnDefendButtonPressed()
	{
		if (_currentTurn != Turn.Player || _isAttacking) return;
		if (_turnsOfDefense > 0)
		{
			SendMessageForAction("You cannot defend while you are using a potion of defense already.");
			return;
		}

		var action = Action.Defend;
		SendMessageForAction("The Player decided to defend himself from the enemy! Next turn.");
		_currentAction = action;
		_currentTurn = Turn.Enemy;

		if (_currentEnemyAction == EnemyAction.Defend)
			_currentEnemyAction = EnemyAction.Nothing;

		await ToSignal(GetTree().CreateTimer(5.0f), Timer.SignalName.Timeout);
		
		EnemyTurn();
	}

	private void OnFleeButtonPressed()
	{
		if (_currentTurn != Turn.Player || _isAttacking) return;
		LeaveBattle();
	}

	private void EnemyTurn()
	{
		switch (_enemy.Name)
		{
			case "Slime":
				SlimeTurn();
				break;
			case "Orc":
				OrcTurn();
				break;
			case "Creeper":
				CreeperTurn();
				break;
			case "Goblin":
				GoblinTurn();
				break;
			case "BDemon":
				BDemonTurn();
				break;
			case "HDemon":
				HDemonTurn();
				break;
			case "Boss":
				BossTurn();
				break;
		}
		
	}

	private void PlayerTurn()
	{
		if (_currentAction != Action.Defend) _currentAction = Action.Nothing;
		_turnsOfDefense--;
		_turnsOfAttack--;
		_currentTurn = Turn.Enemy;
		EnemyTurn();
	}

	private void EnemyTurnEnd()
	{
		if (_currentEnemyAction != EnemyAction.Defend) _currentEnemyAction = EnemyAction.Nothing;
		if (_shielding) _shielding = false;
		_currentTurn = Turn.Player;
		if (_usedPoison)
		{
			_enemy.Hurt(3);
			SendMessageForAction("The " + _enemy.Name + " was damaged by the poison!");
			HandleEnemyDeath();
		}
	}

	private void OnCloneButtonPressed(string name)
	{
		foreach (var slot in _player.Slots)
		{
			if (slot.SlotName != name) continue;
			if (slot.ItemRef == null) return;

			switch (slot.ItemRef.ItemType)
			{
				case Item.Types.Food:
					if (_player.GetHealth() == _player.MaxHealth)
					{
						SendMessageForAction("You cannot eat while you are at full health.");
						return;
					}
					var actualHeal = Math.Clamp(slot.ItemRef.Heal, 0, _player.MaxHealth - _player.GetHealth());
					
					_currentAction = Action.Eat;
					SendMessageForAction("The player ate " + slot.ItemName + " and gained " + actualHeal + " health, it is now the enemies turn.");
					
					var damagePop = GetNode<DamagePopUp>("DamagePopUp");
					damagePop.GlobalPosition = _player.GlobalPosition + new Vector2((float)GD.RandRange(-10.0, 10.0), -40);
					damagePop.Init((int)actualHeal, false);
					
					_player.Eat(slot.ItemName, slot.ItemRef);
					UpdateInventory();
					
					TurnEnd();
					
					break;
				case Item.Types.Weapon:
					if (_isAttacking) return;
					_currentAction = Action.Attack;
					_playerDamage = slot.ItemRef.Damage;
					_inventoryClone.Visible = false;
					PlayerAttack(slot.ItemName);
					break;
				case Item.Types.Usable:
					if (_isAttacking) return;
					_currentAction = Action.Eat;
					QueryUsable(slot.ItemName, slot.ItemRef);
					break;
				default:
					_inventoryClone.Visible = false;
					SendMessageForAction("This item cannot aid you in this battle, you must make better decisions.");
					TurnEnd();
					break;
			}
		}
	}
	
	private async void SendMessageForAction(string action)
	{
		var actionMenu = GetNode<TextMenu>("BattleUI/MessageBox/ActionMenu");
		var message = new Array<string>()
		{
			action
		};
		
		actionMenu.ChangeMessages(message);
		actionMenu.Init();
		
		await ToSignal(GetTree().CreateTimer(2.0f), Timer.SignalName.Timeout);
	}

	private void UpdateInventory()
	{
		_inventoryClone.QueueFree();
		var inventoryClone = _player.GetNode<Control>("Inventory").Duplicate() as Control;
		if (inventoryClone == null)
		{
			GD.PrintErr("[ERROR] : BATTLE : Player lacks inventory.");
		}
		else
		{
			AddChild(inventoryClone);
			inventoryClone.GlobalPosition = _player.GlobalPosition;
			_inventoryClone = inventoryClone;
		}
		
		SetUpInventoryButtons();
	}

	private void SetUpInventoryButtons()
	{
		var slots = _inventoryClone.GetNode<GridContainer>("NinePatchRect/Slots");

		foreach (var slot in slots.GetChildren())
		{
			var button = slot.GetNode<Button>("Button");
			button.FocusMode = Control.FocusModeEnum.None;
			button.Pressed += () => OnCloneButtonPressed(slot.Name);
		}
	}

	private void PlayerAttack(string weapon)
	{
		if (weapon == "chipped" || weapon == "strongerSword" || weapon == "finalSword")
		{
			_isAttacking = true;
			_initialPlayerPos =  _player.GlobalPosition;
			_player.ToggleAttack();
		}
		else if (weapon == "bow")
		{
			_player.ToggleAttack();
			_isAttackingWithBow = true;
		}
	}

	private async void DamageEnemy(int dmg, string anim, string hitMessage)
	{
		
		if (_fightingGoblin && _currentTurn == Turn.Player && !_isAttackingWithBow)
		{
			_player.Hurt(_enemy.Damage / 2);
			SendMessageForAction("The Goblin hurt you with its spear when you got close!");
			await ToSignal(GetTree().CreateTimer(2.0f), Timer.SignalName.Timeout);
		}


		if (!_isAttackingWithBow)
		{
			var anims = (_currentTurn == Turn.Player)
				? _player.GetNode<AnimatedSprite2D>("Anims")
				: _enemy.GetNode<AnimatedSprite2D>("Sprite");
			anims.Play(anim);
		}

		if (dmg > 0)
		{
			if (_currentTurn == Turn.Player)
			{
				_enemy.Hurt(dmg);
				if (_enemy.Name == "BDemon")
				{
					_enemy.Damage *= 2;
					SendMessageForAction("The blood demon absorbed its blood and its attack sharply rose!");
				}
			}
			else
			{
				_player.Hurt(dmg);
			}
		}
		
		
		var damagePop = GetNode<DamagePopUp>("DamagePopUp");
		if (_currentTurn == Turn.Player)
		{
			damagePop.GlobalPosition = _enemy.GlobalPosition;
			damagePop.Init(dmg, true, hitMessage);
		}
		else
		{
			damagePop.GlobalPosition = _player.GlobalPosition;
			damagePop.Init(dmg, true, hitMessage);
		}
		
		HandleEnemyDeath();
		
	}

	private async void HandleEnemyDeath()
	{
		if (_enemy.GetHealth() <= 0)
		{

			var actionPanel = GetNode<PanelContainer>("BattleUI/ActionPanel");
			actionPanel.Visible = false;
			_enemy.ProcessMode = ProcessModeEnum.Disabled;
			SendMessageForAction("The " + _enemy.Name + " has been overwhelmed and the player has won.");
			_currentTurn = Turn.Player;
			SetPhysicsProcess(false);

			await ToSignal(GetTree().CreateTimer(3.0f), Timer.SignalName.Timeout);
			
			if (_isAttacking || _isAttackingWithBow)
			{
				_player.ToggleAttack();
			}
			
			_enemy.QueueFree();
			_currentEnemyAction = EnemyAction.Dead;
			LeaveBattle();
		}
	}
	
	private void CalculateAndAttack()
	{
		var rnd = GD.RandRange(0, 100) % _anims.Count;
		var anim = _anims[rnd];
		
		if (_isAttackingWithBow) anim = "shoot";
		
		var dmg = (_currentTurn == Turn.Player) ? _playerDamage : _enemy.Damage;
		
		var hitMessage = "Good Hit!";

		if (_currentTurn == Turn.Enemy &&
		    (_enemy.Name == "Slime" || _enemy.Name == "Creeper" || _enemy.Name == "Goblin" || _enemy.Name == "Boss"))
		{
			anim = "attack";
			dmg = _enemy.Damage;
		}

		if (_turnsOfAttack > 0)
			dmg *= 2;

		if (_currentTurn == Turn.Enemy && (_currentAction == Action.Defend || _turnsOfDefense > 0))
		{
			_currentAction = Action.Nothing;
			dmg /= 2;
		}

		if (_currentTurn == Turn.Player && _currentEnemyAction == EnemyAction.Defend)
		{
			_currentEnemyAction = EnemyAction.Nothing;
			dmg /= 2;
		}

		if (_shielding && _turnsOfShield > 0)
		{
			dmg = 0;
			_shielding = false;
		}
		
		rnd = GD.RandRange(0, 9);
		switch (rnd)
		{
			case 0 or 5 or 6 or 7 or 8 or 9:
				break;
			case 1:
				dmg += 1;
				hitMessage = "Strong Hit!";
				break;
			case 2:
				dmg -= 1;
				hitMessage = "Weak Hit!";
				break;
			case 3:
				dmg *= 2;
				hitMessage = "Very Strong Hit!";
				break;
			case 4:
				dmg /= 2;
				hitMessage = "Very Weak Hit!";
				break;
		}

		rnd = (_isAttackingWithBow) ? GD.RandRange(1, 6) : GD.RandRange(0, 15);
		
		if (rnd == 7)
		{
			dmg = 0;
			hitMessage = "Miss! Ouch!";
		}
		
		DamageEnemy(dmg, anim, hitMessage);
	}

	private void HealEnemy()
	{
		var enemySprite = _enemy.GetNode<AnimatedSprite2D>("Sprite");
		var rnd = GD.RandRange(0, 3);
		int heal = 0;
		switch (rnd)
		{
			case 0:
				heal = (int)_originalEnemyHp / 3;
				break;
			case 1 or 2 or 3:
				heal = (int)_originalEnemyHp / 2;
				break;
		}

		_enemy.ChangeHealth(heal);

		enemySprite.Play("heal");
		
		var damagePop = GetNode<DamagePopUp>("DamagePopUp");
		damagePop.GlobalPosition = _enemy.GlobalPosition + new Vector2((float)GD.RandRange(-10.0, 10.0), -40);
		damagePop.Init(heal, false);
		
				
		SendMessageForAction("The " + _enemy.Name + " decided to heal itself! It gained " + heal + " health! Next turn.");
		
		TurnEnd();
		
		ToggleStopEnemyProcess();
	}

	private bool _stop;
	private void ToggleStopEnemyProcess()
	{
		_stop = !_stop;
		_enemy.StopProcess(_stop);
	}

	private bool GiveDefense()
	{
		if (_usedSpecialDefense)
		{
			SendMessageForAction("This item can only be used once per battle.");
			return false;
		}
		_turnsOfDefense = 3;
		_usedSpecialDefense = true;
		SendMessageForAction("You used a defensive potion and were granted two turns of increased defense.");
		_inventoryClone.Visible = false;
		return true;
	}

	private void QueryUsable(string name, Item itemRef)
	{
		if (name == "slime")
		{
			if (!GiveDefense()) return;
			_player.Eat(name, itemRef);
			UpdateInventory();
			TurnEnd();
		}
		else if (name == "rage")
		{
			if (_usedSpecialAttack)
			{
				SendMessageForAction("This item can only be used once per battle.");
				return;
			}
			_usedSpecialAttack = true;
			_turnsOfAttack = 3;
			SendMessageForAction("You used a rage potion and were granted two turns of increased attack.");
			_inventoryClone.Visible = false;
			
			_player.Eat(name, itemRef);
			UpdateInventory();
			
			TurnEnd();
		}
		else if (name == "health")
		{
			_player.IncreaseMaxHealth(3);
			_inventoryClone.Visible = false;
			_player.Eat(name, itemRef);
			SendMessageForAction("The player consumed a health potion, increasing their max health and healing them.");
			UpdateInventory();
			TurnEnd();
		}
		else if (name == "shield")
		{
			if (_turnsOfShield <= 0)
			{
				SendMessageForAction("The shield is too damaged to be used again this battle.");
				return;
			}
			SendMessageForAction("The player used a shield to protect themselves from incoming attacks!");
			_shielding = true;
			_turnsOfShield--;
			_inventoryClone.Visible = false;
			TurnEnd();
		}
		else if (name == "poison")
		{
			if (_usedPoison) return;
			_usedPoison = true;
			_player.Eat(name, itemRef);
			SendMessageForAction("The player splashed poison on the opponent!");
			UpdateInventory();
			TurnEnd();
		}
	}

	private bool _turnEndPending;
	private async void TurnEnd(float amountOfWait = 4.0f)
	{
		if (_turnEndPending) return;
		_turnEndPending = true;

		if (_currentTurn == Turn.Enemy) amountOfWait = 1.5f;
		
		await ToSignal(GetTree().CreateTimer(amountOfWait), Timer.SignalName.Timeout);

		_turnEndPending = false;
		
		if (_currentTurn == Turn.Player)
			PlayerTurn();
		else
			EnemyTurnEnd();
	}


	private void SlimeTurn()
	{
		var rnd = GD.RandRange(0, 10);
		switch (rnd)
		{
			case 0 or 1 or 2 or 3 or 4 or 5:
				
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Attack;
				_initialEnemyPos = _enemy.GlobalPosition;
				_enemyIsAttacking = true;
				break;
			case 6 or 7 or 8:
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Special;
				HealEnemy();
				break;
			case 9 or 10:
				_currentEnemyAction = EnemyAction.Defend;
				SendMessageForAction("The " + _enemy.Name + " decided to defend! Next turn.");
				TurnEnd();
				break;
			
		}
	}

	private async void OrcTurn()
	{
		var enemySprite = _enemy.GetNode<AnimatedSprite2D>("Sprite");
		var rnd = GD.RandRange(0, 7);

		switch (_maxFireUp)
		{
			case 4:
				rnd = GD.RandRange(5, 7);
				break;
			case 3:
				rnd = GD.RandRange(3, 6);
				break;
			case 2 or 1:
				rnd = GD.RandRange(0, 5);
				break;
			case 0:
				rnd = GD.RandRange(0, 4);
				break;
		}
		
		switch (rnd)
		{
			case 1 or 2 or 3:
				
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Attack;
				_initialEnemyPos = _enemy.GlobalPosition;
				_enemyIsAttacking = true;
				break;
			case 5 or 6 or 7:
				_maxFireUp--;
				
				_currentEnemyAction = EnemyAction.Special;
				
				ToggleStopEnemyProcess();
				
				enemySprite.Play("rage");
				SendMessageForAction("The Orc fired itself up! The Orc's attack sharply increased!");

				await ToSignal(enemySprite, AnimationMixer.SignalName.AnimationFinished);
				
				_enemy.Damage *= 3;
				ToggleStopEnemyProcess();
				
				TurnEnd();
				break;
			case 4 or 0:
				_currentEnemyAction = EnemyAction.Defend;
				SendMessageForAction("The " + _enemy.Name + " decided to defend! Next turn.");
				TurnEnd();
				break;
				
		}
	}


	private bool _stolen = true;
	private void CreeperTurn()
	{

		var slots = ConstructSlotArray();
		if (slots.Count <= 0) _stolen = false;
		
		var rnd = GD.RandRange(1, 10);
		if (!_stolen)
		{
			rnd =  GD.RandRange(1, 5);
		}
		
		switch (rnd)
		{
			case 1 or 2 or 3 or 4:
				
				ToggleStopEnemyProcess();                                  
				_currentEnemyAction = EnemyAction.Attack;
				_initialEnemyPos = _enemy.GlobalPosition;
				_enemyIsAttacking = true;
				break;
			case 6 or 7 or 8 or 9 or 10:
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Special;
				StealItem(slots);
				break;
			case 5:
				_currentEnemyAction = EnemyAction.Defend;
				SendMessageForAction("The " + _enemy.Name + " decided to defend! Next turn.");
				TurnEnd();
				break;
		}
		
	}

	private void GoblinTurn()
	{
		var rnd = GD.RandRange(0, 7);
		switch (rnd)
		{
			case 0 or 1 or 2 or 3 or 4 or 5:
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Attack;
				_initialEnemyPos = _enemy.GlobalPosition;
				_enemyIsAttacking = true;
				break;
			case 6 or 7:
				_currentEnemyAction = EnemyAction.Defend;
				SendMessageForAction("The " + _enemy.Name + " decided to defend! Next turn.");
				TurnEnd();
				break;
			
		}
	}

	private void BDemonTurn()
	{
		var rnd = GD.RandRange(0, 7);
		switch (rnd)
		{
			case 0 or 1 or 2 or 3 or 4 or 5:
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Attack;
				_initialEnemyPos = _enemy.GlobalPosition;
				_enemyIsAttacking = true;
				break;
			case 6 or 7:
				_currentEnemyAction = EnemyAction.Defend;
				SendMessageForAction("The " + _enemy.Name + " decided to defend! Next turn.");
				TurnEnd();
				break;
		}
	}
	
	private void HDemonTurn()
	{
		var rnd = GD.RandRange(0, 7);
		switch (rnd)
		{
			case 0 or 1 or 2 or 3 or 4 or 5:
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Attack;
				_initialEnemyPos = _enemy.GlobalPosition;
				_enemyIsAttacking = true;
				break;
			case 6 or 7:
				_currentEnemyAction = EnemyAction.Defend;
				SendMessageForAction("The " + _enemy.Name + " decided to defend! Next turn.");
				
				var rand = GD.RandRange(0, 1);
				if (rand == 1) TurnEnd();
				else
				{
					SendMessageForAction("The Horn Demon used its speed to make another move!");
					HDemonTurn();
				}
				
				break;
		}
	}

	private async void BossTurn()
	{
		var rnd = GD.RandRange(1, 4);
		var enemySprite = _enemy.GetNode<AnimatedSprite2D>("Sprite");
		
		switch (rnd)
		{
			case 1 or 2:
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Attack;
				_initialEnemyPos = _enemy.GlobalPosition;
				_enemyIsAttacking = true;
				break;
			case 3:
				ToggleStopEnemyProcess();
				_currentEnemyAction = EnemyAction.Special;
				HealEnemy();
				break;
			case 4:
				_currentEnemyAction = EnemyAction.Special;
				
				ToggleStopEnemyProcess();
				
				enemySprite.Play("hurt");
				SendMessageForAction("He doubled his attack damage!");

				await ToSignal(enemySprite, AnimationMixer.SignalName.AnimationFinished);
				
				_enemy.Damage *= 2;
				ToggleStopEnemyProcess();
				
				TurnEnd();
				break;
		}
	}
	
	

	private void StealItem(Array<InventorySlot> slots)
	{
		var rnd = GD.RandRange(50, 100) % slots.Count;
		var slot = slots[rnd];
		
		GD.Print(slot.ItemName);
		
		
		if (slot.Count <= 1)
		{
			_player.Eat(slot.ItemName, slot.ItemRef, "heal");
			SendMessageForAction("The Creeper took one of your items!");
		}
		else
		{
			_player.Eat(slot.ItemName, slot.ItemRef, "heal");
			SendMessageForAction("The Creeper took one out of a stack of your items!");
		}
		UpdateInventory();
		ToggleStopEnemyProcess();
		TurnEnd();
	}

	private Array<InventorySlot> ConstructSlotArray()
	{
		var inventorySlots = new Array<InventorySlot>();
		
		foreach (var slot in _player.Slots)
		{
			if (slot.ItemRef == null) continue;
			
			if (slot.ItemName != "chipped" && slot.ItemName != "strongerSword" && slot.ItemName != "finalTorch" && slot.ItemName != "torch" && slot.ItemName != "shield" && slot.ItemName != "bow")
			{
				inventorySlots.Add(slot);
			}
		}
		return inventorySlots;
	}
	
}
