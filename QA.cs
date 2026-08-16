using Godot;

[GlobalClass]
public partial class QA : Resource
{
    [Export] public string Question { get; set; } = "";
    [Export] public string Answer { get; set; } = "";
}