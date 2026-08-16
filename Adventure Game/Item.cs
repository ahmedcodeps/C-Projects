using Godot;
using Godot.Collections;
using System;

public partial class Item
{

	public enum Types
	{
		Food,
		Weapon,
		Usable,
		Equippable,
	}
	
	[Export] public Texture2D Texture;
	[Export] public string ItemName;
	[Export] public Types ItemType;
	[Export] public int Heal;
	[Export] public int Damage;
	[Export] public string ItemDescription;
	[Export] public Array<string> Messages;
		
	private string _name;
	private Types _type;
	private int _heal;
	private int _damage;
	private string _description;
	public Texture2D Tex;

	private bool _canGoBack;
	private bool _equipped;

}
public partial class Item : Area2D
{
	// Called when the node enters the scene tree for the first time.
	public override void _Ready()
	{
		_name = ItemName;
		_type = ItemType;
		_heal = Heal;
		_damage = Damage;
		_description = ItemDescription;
		Tex = Texture;

		GetNode<TextMenu>("TextMenu").ChangeMessages(Messages); 
		GetNode<Sprite2D>("Sprite").Texture = Texture;
		GetNode<Label>("ItemMenu/Name/Label").Text = _name;

		GetNode<Button>("ItemMenu/Use").Pressed += OnUsePressed;
		GetNode<Button>("ItemMenu/Eat").Pressed += OnEatPressed;
		GetNode<Button>("ItemMenu/Description").Pressed += OnDescriptionPressed;
		GetNode<Button>("ItemMenu/Drop").Pressed += OnDropPressed;
		GetNode<Button>("ItemMenu/Equip").Pressed += OnEquipPressed;
		GetNode<Button>("ConfirmationMenu/Yes").Pressed += OnYesPressed;
		GetNode<Button>("ConfirmationMenu/No").Pressed += OnNoPressed;
		
	}
	
	
	public override void _Process(double delta)
	{
		var canvasLayer = GetNode<CanvasLayer>("ItemMenu");
		var eventContainer = GetNode<HBoxContainer>("ItemMenu/MarginContainer/AnswerContainer/EventContainer");
		var eventLabel = eventContainer.GetNode<Label>("Event");
		if (_canGoBack)
		{
			if (Input.IsActionPressed("back"))
			{
				_canGoBack = false;
				eventContainer.Visible = false;
				foreach (var node in canvasLayer.GetChildren())
				{
					if (node is Button button) button.Visible = true;
				}

				eventLabel.VisibleRatio = 0;
				eventLabel.Text = "";
			}
		}

		if (GetParent().GetParent() is Player)
		{
			var inventory = GetParent().GetParent().GetNode<Control>("Inventory");
			if (!inventory.Visible) canvasLayer.Visible = false;
		}
	}

	private void OnBodyEntered(Node2D body)
	{
		if (body is Player player)
		{
			player.AddItem(_name, Tex, this);
			Visible = false;
			CallDeferred("reparent", body.GetNode<Node2D>("Items"));
			var collision = GetNode<CollisionShape2D>("CollisionShape2D");
			collision.SetDeferred("disabled", true);
			GetNode<TextMenu>("TextMenu").Init();
			
			if (_name == "strongerSword" || _name == "finalTorch" || _name == "finalSword")
			{
				foreach (var slot in player.Slots)
				{
					if (_name == "strongerSword")
					{
						if (slot.ItemRef.ItemName == "chipped")
							player.Eat(slot.ItemRef.ItemName, slot.ItemRef);
					}
					else if (_name == "finalTorch")
					{
						if (slot.ItemRef.ItemName == "torch")
						{
							player.Eat(slot.ItemRef.ItemName, slot.ItemRef);
							
							if (_createdTorch)
							{
								_createdTorch = false;
								player.GetNode<PointLight2D>("torch").Visible = false;
							}
						}
					}
					else if (_name == "finalSword")
					{
						if (slot.ItemRef.ItemName == "chipped" || slot.ItemRef.ItemName == "strongerSword")
							player.Eat(slot.ItemRef.ItemName, slot.ItemRef);
					}
				}
			}
		}
	}

	
	private void CreateTorch()
	{
		var player = GetTree().GetFirstNodeInGroup("Player") as Player;
		
		if (player == null)
		{
			GD.PrintErr("[ERROR] : ITEM : Player cannot be found.");
			return;
		}
		
		foreach (var light in player.GetChildren())
		{
			if (light is PointLight2D)
				light.QueueFree();
		}

		var torch = GetNode<PointLight2D>("TorchLight");
		
		var newTorch = torch.Duplicate() as PointLight2D;
		
		if (newTorch == null)
		{
			GD.PrintErr("[ERROR] : ITEM : Torch cannot be found.");
			return;
		} 
		
		newTorch.Visible = true;
		newTorch.Name = "torch";
		
		player.AddChild(newTorch);
	}

	public void Menu()
	{
		var canvasLayer = GetNode<CanvasLayer>("ItemMenu");
		var player = GetParent().GetParent() as Player;
		canvasLayer.Visible = !canvasLayer.Visible;
		if (canvasLayer.Visible) player?.ChangeState(Player.PlayerState.InMenu);
	}

	private void OnUsePressed()
	{
		var player = GetParent().GetParent() as Player;

		if (player == null)
		{
			GD.PrintErr("[ERROR] : ITEM : Player cannot be found.");
			return;
		}

		if (_type == Types.Usable)
		{

			switch (_name)
			{
				case "slime" or "rage" or "poison":
					SendMessage("This item can only be used during battle. Using it now would be futile.");
					break;
				case "health":
					player.IncreaseMaxHealth(3);
					player.Eat(_name, this);
					SendMessage(
						"You have consumed a health potion, increasing your max health and healing yourself. You are now at " +
						player.MaxHealth + " health.");
					break;
				default:
					SendMessage(_name + " cannot be used at this time or is not a usable item.");
					break;
			}
		}
	}

	private void OnEatPressed()
	{
		var player = GetParent().GetParent() as Player;
		var canvasLayer = GetNode<CanvasLayer>("ItemMenu");
		var textMenu  = GetNode<TextMenu>("TextMenu");
		
		if (_type == Types.Food)
		{
			if (player?.GetHealth() == player?.MaxHealth)
			{
				SendMessage("You cannot eat while at full health, could there be a way to increase maximum health?");
				return;
			}
			canvasLayer.Visible = false;
			player.Eat(_name, this);
			
			var newMessage = new Array<String>()
			{
				"You ate " + _name + " and gained " + _heal + " health. You are now at " + player.GetHealth() + " health."
			};
			
			textMenu.ChangeMessages(newMessage);
			textMenu.Init();
		}
		else
			SendMessage(_name + " Cannot be eaten as it would likely cause instant death or violent vomiting.");
	}
	
	private static bool _createdTorch;
	
	private void OnEquipPressed()
	{
		var player = GetParent().GetParent() as Player;
		var canvasLayer = GetNode<CanvasLayer>("ItemMenu");
		var equipLabel = canvasLayer.GetNode<Button>("Equip");

		if (player == null)
		{
			GD.PrintErr("[ERROR] : ITEM : Player cannot be found.");
			return;
		}
		
		if (_type == Types.Equippable)
		{

			if (!_createdTorch)
			{
				_createdTorch = true;
				CreateTorch();
			}

			PointLight2D torch = null;
			foreach (var item in player.GetChildren())
			{
				if (item is PointLight2D pointLight)
					torch = pointLight;
			}
			
			_equipped = !_equipped;	
			
			equipLabel.Text = _equipped ? "Unequip" : "Equip";

			if (_equipped && torch != null)
				torch.Visible = true;
			else if (torch != null)
				torch.Visible = false;
			
			
		}
		else
			SendMessage(_name + " Cannot be equipped as it would bring unnecessary weight to carry.");
	}

	private void OnDescriptionPressed()
	{
		if (_description != string.Empty)
			SendMessage(_description);
		else
		{
			SendMessage(_name + " you cannot figure out what this is...");
		}
	}

	private void OnDropPressed()
	{
		var canvasLayer = GetNode<CanvasLayer>("ItemMenu");
		var confirmationMenu = GetNode<CanvasLayer>("ConfirmationMenu");
		var confirmationLabel = confirmationMenu.GetNode<Label>("MarginContainer/ConfirmationContainer/EventContainer/Event");
		canvasLayer.Visible = false;
		confirmationMenu.Visible = true;
		
		var tween = CreateTween();
		tween.TweenProperty(confirmationLabel, "visible_ratio", 1.0,confirmationLabel.Text.Length * 0.05);
	}

	private void OnYesPressed()
	{
		var player = GetParent().GetParent() as Player;
		var confirmationMenu = GetNode<CanvasLayer>("ConfirmationMenu");
		var confirmationLabel = confirmationMenu.GetNode<Label>("MarginContainer/ConfirmationContainer/EventContainer/Event");
		
		confirmationLabel.VisibleRatio = 0;
		confirmationMenu.Visible = false;

		if (player == null)
		{
			GD.PrintErr("[ERROR] : ITEM : Player cannot be found.");
			return;
		}
		
		foreach (var slot in player.Slots)
		{
			if (slot.ItemName == _name && slot.Count > 1)
			{
				player.RemoveOne(slot, true);
				return;
			}
		}
		
		if (_equipped) OnEquipPressed();
		player.RemoveItem(_name);
	}

	private void OnNoPressed()
	{
		var confirmationMenu = GetNode<CanvasLayer>("ConfirmationMenu");
		var confirmationLabel = confirmationMenu.GetNode<Label>("MarginContainer/ConfirmationContainer/EventContainer/Event");
		var canvasLayer = GetNode<CanvasLayer>("ItemMenu");
		
		confirmationLabel.VisibleRatio = 0;
		confirmationMenu.Visible = false;
		canvasLayer.Visible = true;
	}

	private void SendMessage(string message)
	{
		var canvasLayer = GetNode<CanvasLayer>("ItemMenu");
		var eventContainer = GetNode<HBoxContainer>("ItemMenu/MarginContainer/AnswerContainer/EventContainer");
		var eventLabel = eventContainer.GetNode<Label>("Event");

		foreach (var node in canvasLayer.GetChildren())
		{
			if (node is Button button) button.Visible = false;
		}
		
		eventContainer.Visible = true;
		eventLabel.Text = message;
		var tween = CreateTween();
		tween.TweenProperty(eventLabel, "visible_ratio", 1.0,eventLabel.Text.Length * 0.05);
		tween.Finished += TweenOnFinished;
	}

	private void TweenOnFinished()
	{
		_canGoBack = true;
	}
	


}
