using Godot;
using Godot.Collections;	
using System;

public partial class Game : Node2D
{
	[Export] private Array<PackedScene> Maps { get; set; }
	[Export] private PackedScene BattleScene { get; set; }

	private Dictionary<int, Node2D> _maps =  new Dictionary<int, Node2D>();
	
	private int _currentMap = 0;
}
public partial class Game : Node2D
{
	// Called when the node enters the scene tree for the first time.
	public override void _Ready()
	{
		CallDeferred(nameof(AddMap));
		
	}
	
	private void SetUp()
	{
		var player = GetNode<CharacterBody2D>("Entities/Player");
		var map = GetNode<Node2D>("Map").GetChild(0);
		var spawnPoint = map.GetNode<Marker2D>("SpawnPoint");
		var mapMenu = map.GetNode<TextMenu>("MapMenu");
		
		player.GlobalPosition = spawnPoint.GlobalPosition;
		//player.GetNode<AnimatedSprite2D>("Anims").Play("wake");
		
		mapMenu.Init();
		map._Ready();
	}

	private void DisableCollisions()
	{
		foreach (var map in GetNode<Node2D>("Map").GetChildren())
		{
			foreach (var layer in map.GetNode<Node2D>("Layers").GetChildren())
			{
				if (layer is TileMapLayer tileMapLayer)
				{
					tileMapLayer.CollisionEnabled = false;
				}
			}
		}
	}

	private static void EnableCollisions(Node2D map)
	{
		foreach (var layer in map.GetNode<Node2D>("Layers").GetChildren())
		{
			if (layer is TileMapLayer tileMapLayer)
			{
				tileMapLayer.CollisionEnabled = true;
			}
		}
	}

	public void SwitchMap(int map)
	{
		CallDeferred(nameof(RemoveMap));
		_currentMap = map;
		_Ready();
	}

	private void AddMap()
	{
		
		
		if (_maps.TryGetValue(_currentMap, out var map))
			map = _maps[_currentMap];
		else
		{
			map = Maps[_currentMap].Instantiate<Node2D>();
			_maps[_currentMap] = map;
		}
		
		GetNode<Node2D>("Map").AddChild(map);
		SetUp();
	}

	private void RemoveMap()
	{
		var map = GetNode<Node2D>("Map").GetChild(0);
		if (map.HasMethod("DeleteDetectionArea"))
			map.Call("DeleteDetectionArea");
		
		GetNode<Node2D>("Map").RemoveChild(map);
	}

	public void InstantiateBattle()
	{
		var battle = BattleScene.Instantiate<Battle>();
		var battleNode = GetNode<Node2D>("Battle");
		battleNode.Call("add_child", battle);
	}

	
}
