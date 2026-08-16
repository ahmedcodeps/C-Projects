using Godot;
using Godot.Collections;

[GlobalClass]
public partial class DialogueData : Resource
{
    [Export] public Array<QA> Entries { get; set; } = new Array<QA>();
}

