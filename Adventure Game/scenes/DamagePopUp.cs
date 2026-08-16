using Godot;

public partial class DamagePopUp : Node2D
{
	public void Init(int damage, bool damageOrHeal, string hitMessage = null)
	{
		var label = GetNode<Label>("Label");
		var label2 = GetNode<Label>("Label2");

		if (damageOrHeal)
		{
			
			label2.Text = hitMessage;
			label2.Visible = true;
			label2.Modulate = new Color(1, 1, 1);
		}

		label.Modulate = new Color(1, 1, 1);
		
		label.Text = damageOrHeal ?  "-" + damage : "+" + damage;
		if (damage == 0 && hitMessage != "Miss! Ouch!") label.Text = "No damage!";
		
		if (damageOrHeal)
		{
			label.AddThemeColorOverride("font_color", new Color(1f, 0.3f, 0.3f));
		}
		else
		{
			label.AddThemeColorOverride("font_color", new Color(0, 1, 0));
		}
		

		
		label.Visible = (damage > 0);
		var tween = CreateTween();
		tween.SetParallel();

		tween.TweenProperty(this, "position", Position + new Vector2(0, -40), 0.8f).SetEase(Tween.EaseType.Out);
		tween.TweenProperty(label, "modulate:a", 0.0f, 0.8f).SetDelay(0.2f);
		tween.TweenProperty(label2, "modulate:a", 0.0f, 0.8f).SetDelay(0.2f);
		
		tween.Chain().TweenCallback(Callable.From(() =>
		{
			label.Visible = false;
			label2.Visible = false;
		}));
	}
	
	
}
