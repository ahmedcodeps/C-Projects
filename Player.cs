using Godot.Collections;
using Godot;
using System;

public partial class Player
{
	
	
	private const int Speed = 100;
	private Vector2 _direction = Vector2.Zero;
	private float _health = 10;
	public float MaxHealth = 10;

	public Array<InventorySlot> Slots = new Array<InventorySlot>
	{
		new InventorySlot("Slot1", "empty", 0),
		new InventorySlot("Slot2", "empty", 0),
		new InventorySlot("Slot3", "empty", 0),
		new InventorySlot("Slot4", "empty", 0),
		new InventorySlot("Slot5", "empty", 0),
		new InventorySlot("Slot6", "empty", 0),
		new InventorySlot("Slot7", "empty", 0),
		new InventorySlot("Slot8", "empty", 0),
		new InventorySlot("Slot9", "empty", 0),
		new InventorySlot("Slot10", "empty", 0),
		new InventorySlot("Slot11", "empty", 0),
		new InventorySlot("Slot12", "empty", 0),
		new InventorySlot("Slot13", "empty", 0),
		new InventorySlot("Slot14", "empty", 0),
		new InventorySlot("Slot15", "empty", 0),
	};
	

	public enum PlayerState
	{
		Idle,
		Walking,
		InMenu,
		Battling,
		Dead,
	}


	private bool _isAttacking;
	private bool _isHurting;
	public PlayerState CurrentState = PlayerState.Idle;
}

public partial class Player : CharacterBody2D
{
	public override void _PhysicsProcess(double delta)
	{
		var anims = GetNode<AnimatedSprite2D>("Anims");
		
		if (CurrentState == PlayerState.InMenu || CurrentState == PlayerState.Battling && !_isAttacking)
		{
			anims.Play(_isHurting ?  "hurt" : "idle");
			if (CurrentState == PlayerState.Battling) anims.FlipH = true;
			return;
		}

		if (CurrentState == PlayerState.Battling && _isAttacking)
		{
			return;
		}

		if (CurrentState == PlayerState.Dead)
		{
			return;
		}
		
		
		_direction = Input.GetVector("left", "right", "up", "down");
		Velocity = _direction * Speed;
		
		MoveAndSlide();
		
		if (_direction.X != 0 || _direction.Y != 0)
		{
			CurrentState = PlayerState.Walking;
			anims.Play("walk");
			anims.FlipH = (_direction.X < 0);
		}
		else
		{
			CurrentState = PlayerState.Idle;
			anims.Play("idle");
		}
	}

	public override void _Process(double delta)
	{
		if (Input.IsActionJustPressed("inventory") && CurrentState != PlayerState.Battling)
		{
			var inventory = GetNode<Control>("Inventory");
			inventory.Visible = !inventory.Visible;
			CurrentState = inventory.Visible ? PlayerState.InMenu : PlayerState.Walking;
		}
	}

	public override void _Ready()
	{
		var slots = GetNode<GridContainer>("Inventory/NinePatchRect/Slots");
		foreach (var slot in slots.GetChildren())
		{
			var button = slot.GetNode<Button>("Button");
			button.Pressed += () => OnButtonPressed( slot.Name);
		}
	}

	public void ChangeState(PlayerState state)
	{
		CurrentState = state;
	}

	public void AddItem(string itemName, Texture2D texture, Item item)
	{
		var existSlot = CheckForItemInSlot(itemName);
		if (existSlot != null)
		{
			existSlot.Count++;
			if (existSlot.Count > 1) UpdateSlotCount(existSlot.SlotName, existSlot.Count);
			return;
		}
		
		var newSlot = CheckForItemInSlot("empty");
		UpdateSlot(newSlot.SlotName, texture);
		newSlot.ItemName = itemName;
		newSlot.Count++;
		newSlot.ItemRef = item;
	}

	public void RemoveItem(string itemToRemove)
	{
		var slot = CheckForItemInSlot(itemToRemove);
		Drop(slot.ItemRef);
		Remove(itemToRemove);
	}

	private void UpdateSlotCount(string slotname, int slotcount)
	{
		var grid = CheckForPanel(slotname);
		grid.GetNode<Label>("Count").Text = slotcount.ToString();
	}

	private void UpdateSlot(string slotName, Texture2D texture)
	{
		if (texture == null) GD.Print("Texture is null");
		var grid = CheckForPanel(slotName);
		grid.Visible = true;
		grid.GetNode<Sprite2D>("Item").Texture = texture;
	}

	private void RemoveSlot(string slotName)
	{
		var grid = CheckForPanel(slotName);
		grid.GetNode<Sprite2D>("Item").Texture = null;
		grid.GetNode<Label>("Count").Text = "";
	}

	public void RemoveOne(InventorySlot slot, bool drop, Item skip = null)
	{
		var items = GetNode<Node2D>("Items");
		slot.Count--;

		if (slot.ItemRef == skip)
		{
			foreach (var child in items.GetChildren())
			{
				if (child.Name.ToString().StartsWith(slot.ItemName, StringComparison.OrdinalIgnoreCase)
				    && child is Item candidate
				    && candidate != skip)
				{
					if (drop) Drop(candidate);
					else candidate.QueueFree();
					break;
				}
			}
		}
		else
		{
			if (drop) Drop(slot.ItemRef);
			else slot.ItemRef.QueueFree();

			if (slot.Count > 0)
			{
				foreach (var child in items.GetChildren())
				{
					if (child.Name.ToString().StartsWith(slot.ItemName, StringComparison.OrdinalIgnoreCase)
					    && child is Item remaining
					    && remaining != slot.ItemRef)
					{
						slot.ItemRef = remaining;
						break;
					}
				}
			}
		}
		
		UpdateSlotCount(slot.SlotName, slot.Count);
	}

	public void Eat(string itemName, Item eatenBy, string heal = null)
	{
		var slot = CheckForItemInSlot(itemName);
		if (_health == MaxHealth && eatenBy.ItemType == Item.Types.Food && heal == null) return;

		if (heal == null)
			_health += Math.Clamp(slot.ItemRef.Heal, 0, MaxHealth - _health);
		
		switch (slot.Count)
		{
			case 1:
				Remove(itemName);
				break;
			case > 1:
				RemoveOne(slot, false, eatenBy);
				break;
		}
	}

	private void OnButtonPressed(string slotName)
	{
		var slot = CheckForSlot(slotName);
		slot.ItemRef?.Menu();
	}

	private async void TakeDamage(int amount)
	{
		_health -= amount;
		if (_health <= 0)
		{
			ChangeState(PlayerState.Dead);

			var deathMenu = GetNode<TextMenu>("DeathMenu");
			deathMenu.Init();
			
			
			await ToSignal(GetTree().CreateTimer(4.0f), "timeout");

			if (GetParent() is Battle b)
				b.LeaveBattle(true);
			else
				GD.PrintErr("Player is not a child of battle!");

			if (_isAttacking)
				ToggleAttack();
			_health = MaxHealth / 2;
		}
	}
	

	private void Drop(Item i)
	{
		var dropRange = GD.RandRange(1.2, 1.8);
		i.GlobalPosition = GlobalPosition * (float)dropRange;
		var map = GetParent().GetParent().GetNode<Node2D>("Map").GetChild(0);
		i.Visible = true;
		if (map is Node2D) 
			i.CallDeferred("reparent", map);
		var collision = i.GetNode<CollisionShape2D>("CollisionShape2D");
		collision.SetDeferred("disabled", false);
	}

	private void Remove(string itemName)
	{

		string slotRemove = null;
		var slot = CheckForItemInSlot(itemName);
		if (slot != null)
		{
			slotRemove = slot.SlotName;
			slot.ItemName = "empty";
			slot.Count = 0;
			slot.ItemRef = null;
		}
		
		if (slotRemove != null) RemoveSlot(slotRemove);
	}

	public float GetHealth()
	{
		return _health;
	}

	public void ToggleAttack()
	{
		_isAttacking = !_isAttacking;
	}
	
	

	private InventorySlot CheckForItemInSlot(string itemName)
	{
		foreach (var slot in Slots)
		{
			if (slot.ItemName != itemName) continue;
			return slot;
		}
		return null;
	}

	private InventorySlot CheckForSlot(string slotName)
	{
		foreach (var slot in Slots)
		{
			if (slot.SlotName != slotName) continue;
			return slot;
		}
		return null;
	}

	private Panel CheckForPanel(string slotName)
	{
		var slots = GetNode<GridContainer>("Inventory/NinePatchRect/Slots");
		foreach (var slot in slots.GetChildren())
		{
			if (slot.Name != slotName) continue;
			if (slot is Panel grid)
				return grid;
		}
		return null;
	}

	public async void Hurt(int damage)
	{
		var dead = false;
		var anims = GetNode<AnimatedSprite2D>("Anims");
		
		if (_health - damage <= 0) dead = true;
		TakeDamage(damage);

		if (dead) return;
		
		_isHurting = true;
		
		await ToSignal(anims, AnimationMixer.SignalName.AnimationFinished);
		
		_isHurting = false;
	}

	public void IncreaseMaxHealth(int amount)
	{
		MaxHealth += amount;
		_health = MaxHealth;
	}
	
}
