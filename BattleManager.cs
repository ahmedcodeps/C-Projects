using Godot;
using Godot.Collections;


public partial class BattleManager: Node
{
	private Node _originalParent;
	private Node _originalParentEnemy;
	private Item _enemyDrop;
	private Vector2 _enemyPos;
	private Vector2 _playerPos;
	private bool _dropHandled;
}
public partial class BattleManager
{
	public void StartBattle(Player player, Enemy enemy)
	{
		var game = GetTree().GetFirstNodeInGroup("Game") as Game;

		if (game == null)
			GD.PrintErr("[ERROR] : BATTLE MANAGER : Game is null");
		
		var mapNode = game?.GetNode("Map");
		var battleNode = game?.GetNode("Battle");
		CallDeferred(nameof(DisableMap), mapNode);

		_originalParent = player.GetParent();
		_originalParentEnemy = enemy.GetParent();
		_playerPos = player.GlobalPosition;
		_enemyPos = enemy.GlobalPosition;
		
		player.ChangeState(Player.PlayerState.Battling);
		enemy.ChangeState(Enemy.EnemyState.Battling);

		player.GetNode<AnimatedSprite2D>("Anims").FlipH = true;
		
		CallDeferred(nameof(HandleEnemyDrop), enemy, game);
		
		game?.InstantiateBattle();
		var battle = battleNode?.GetNode<Battle>("Battle");
		battle?.Init(player, enemy);
	}

	public void EndBattle(Battle battle, Player player, Enemy enemy = null, bool playerDead = false)
	{
		_dropHandled = false;
		var game = GetTree().GetFirstNodeInGroup("Game") as Game;
		Node2D mapNode;
		
		if (game == null)
		{
			GD.PrintErr("[ERROR] : BATTLE MANAGER : Game is null");
		}
		{ 
			mapNode = game?.GetNode("Map") as Node2D;
		}
		
		CallDeferred(nameof(EnableMap), mapNode);
		
		Callable.From(() =>
		{
			battle.RemoveChild(player);
			_originalParent.AddChild(player);
			var map = mapNode?.GetChild(0);

			if (playerDead)
			{
				
				var spawnPoint = map?.GetNode<Marker2D>("SpawnPoint");
				var mapMenu = map?.GetNode<TextMenu>("MapMenu");
				
				if (spawnPoint == null)
				{
					GD.PrintErr("SpawnPoint is null");
				}
				else
				{
					player.GlobalPosition = spawnPoint.GlobalPosition;
				}
				
				enemy?.ChangeHealth(enemy.Health - enemy.GetHealth());
				mapMenu?.Init();
			}
			else
				player.GlobalPosition = _playerPos;
			
			map?._Ready();
			
			var layers = map?.GetNode<Node2D>("Layers");
			if (layers != null && layers.HasNode("Exit"))
			{
				layers.GetNode<TileMapLayer>("Exit").Visible = true;
			}
			
			if (enemy != null)
			{
				_enemyDrop.Owner = null;
				_enemyDrop.GetParent().RemoveChild(_enemyDrop);
				enemy.AddChild(_enemyDrop);
				
				battle.RemoveChild(enemy);
				_originalParentEnemy.AddChild(enemy);

				enemy.GlobalPosition = _enemyPos + new Vector2(30, -30);
			}
			else
			{
				_enemyDrop.Visible = true;
				_enemyDrop.GetNode<CollisionShape2D>("CollisionShape2D").Disabled = false;
				_enemyDrop.GlobalPosition = _playerPos;
			}
			
			battle.QueueFree();
			
			player.ChangeState(Player.PlayerState.Idle);
			enemy?.ChangeState(Enemy.EnemyState.Idle);

			ReactivatePlayerCamera(player);
		}).CallDeferred();
		
	}

	private void DisableMap(Node mapNode)
	{
		mapNode.ProcessMode = ProcessModeEnum.Disabled;
	}
	
	private void EnableMap(Node mapNode)
	{
		mapNode.ProcessMode = ProcessModeEnum.Always;
	}
	
	private void ReactivatePlayerCamera(Player player)
	{
		var camera = player.GetNodeOrNull<Camera2D>("Camera2D"); 
		camera?.MakeCurrent();
	}

	private void HandleEnemyDrop(Enemy enemy, Game game)
	{
		if (_dropHandled) return;
		_dropHandled = true;

		var itemArray = new Array<Item>();

		foreach (var node in enemy.GetChildren())
		{
			if (node is Item item)
			{
				item.GetNode<CollisionShape2D>("CollisionShape2D").Disabled = true;
				itemArray.Add(item);
			}
		}

		if (itemArray.Count == 0)
		{
			GD.PrintErr($"{enemy.Name} has no item drops to hand out.");
			return;
		}

		if (itemArray.Count >= 2)
		{
			var rnd = GD.RandRange(1, 10);
			switch (rnd)
			{
				case 1 or 2 or 3:
					rnd = 0;
					break;
				default:
					rnd = 1;
					break;
			}
			
			itemArray[rnd].QueueFree();
			itemArray.RemoveAt(rnd);
		}

		_enemyDrop = itemArray[0];
	
		_enemyDrop.Owner = null;
		_enemyDrop.GetParent().RemoveChild(_enemyDrop);
		var parent = game.GetNode<Node2D>("Map").GetChild(0);
		var items = parent.GetNode<Node2D>("Items");
		items.AddChild(_enemyDrop);
	}
}
