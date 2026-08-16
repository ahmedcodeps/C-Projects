using Godot;

[GlobalClass]
public partial class InventorySlot : Resource
{
    public string SlotName;
    public string ItemName;
    public int Count;
    public Item ItemRef;

    public InventorySlot(string slotName, string itemName, int count)
    {
        SlotName = slotName;
        ItemName = itemName;
        Count = count;
        ItemRef = null;
    }
}