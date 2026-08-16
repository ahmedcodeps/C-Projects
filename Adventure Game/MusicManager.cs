using Godot;

public partial class MusicManager : Node
{
	private AudioStreamPlayer _player;
	private string _currentTrackPath;

	public override void _Ready()
	{
		_player = new AudioStreamPlayer();
		AddChild(_player);
	}

	public void PlayTrack(AudioStream track, float fadeSeconds = 1.0f)
	{
		if (track == null || track.ResourcePath == _currentTrackPath) return;

		_currentTrackPath = track.ResourcePath;

		if (_player.Playing)
		{
			var tween = CreateTween();
			tween.TweenProperty(_player, "volume_db", -40f, fadeSeconds);
			tween.TweenCallback(Callable.From (() => SwapTrack(track, fadeSeconds)));
		}
		else
		{
			SwapTrack(track, fadeSeconds);
		}
	}

	private void SwapTrack(AudioStream track, float fadeSeconds)
	{
		_player.Stream = track;
		_player.VolumeDb = -40f;
		_player.Play();
		
		var tween = CreateTween();
		tween.TweenProperty(_player, "volume_db", 0f, fadeSeconds);
	}

	private void StopMusic(float fadeSeconds = 1.0f)
	{
		var tween = CreateTween();
		tween.TweenProperty(_player, "volume_db", -40f, fadeSeconds);
		tween.TweenCallback(Callable.From(() => _player.Stop()));
	}
}
