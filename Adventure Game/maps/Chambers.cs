using Godot;
using Godot.Collections;
using System;


public partial class Chambers : Node2D
{
	[Export] public AudioStream MapMusic;
	private bool _readNpc;
	private int _totalEnemies = 6;
	private int _maxEnemies = 6;

	public override void _Ready()
	{
		GetNode<MusicManager>("/root/MusicManager").PlayTrack(MapMusic); 
	}

	public override void _Process(double delta)
	{
		var countLabel = GetNode<Label>("EnemyCount/Count");
		countLabel.Text = "Enemies: " + _totalEnemies;

		if (_totalEnemies == 0)
		{
			GetNode<Door>("Door").GlobalPosition = GetNode<Marker2D>("DoorPos").GlobalPosition;
		}
		
	}
	

	private void OnChildExitingTree(Node node)
	{
		if (node is not Enemy) return;
		_totalEnemies--;
	}

	private void OnChildEnteredTree(Node node)
	{
		if (_totalEnemies == _maxEnemies || node is not Enemy) return;
		_totalEnemies++;
	}
}
