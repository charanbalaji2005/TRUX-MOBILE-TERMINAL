package com.meridian.shell.viewer

import android.net.Uri
import androidx.annotation.OptIn
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.media3.common.MediaItem
import androidx.media3.common.Player
import androidx.media3.common.util.UnstableApi
import androidx.media3.exoplayer.ExoPlayer
import androidx.media3.ui.PlayerView
import kotlinx.coroutines.delay
import java.io.File
import java.util.Locale

@OptIn(UnstableApi::class)
@Composable
fun MediaViewer(file: File, isAudioOnly: Boolean = false) {
    val context = LocalContext.current
    var isPlaying by remember { mutableStateOf(false) }
    var currentPos by remember { mutableLongStateOf(0L) }
    var duration by remember { mutableLongStateOf(0L) }
    var isMuted by remember { mutableStateOf(false) }

    val exoPlayer = remember(file) {
        ExoPlayer.Builder(context).build().apply {
            val mediaItem = MediaItem.fromUri(Uri.fromFile(file))
            setMediaItem(mediaItem)
            prepare()
            playWhenReady = true
        }
    }

    DisposableEffect(exoPlayer) {
        val listener = object : Player.Listener {
            override fun onIsPlayingChanged(playing: Boolean) {
                isPlaying = playing
            }

            override fun onPlaybackStateChanged(playbackState: Int) {
                if (playbackState == Player.STATE_READY) {
                    duration = exoPlayer.duration.coerceAtLeast(0L)
                }
            }
        }
        exoPlayer.addListener(listener)

        onDispose {
            exoPlayer.removeListener(listener)
            exoPlayer.release()
        }
    }

    // Polling player position
    LaunchedEffect(isPlaying) {
        while (isPlaying) {
            currentPos = exoPlayer.currentPosition.coerceAtLeast(0L)
            duration = exoPlayer.duration.coerceAtLeast(0L)
            delay(500)
        }
    }

    Column(
        Modifier
            .fillMaxSize()
            .background(Color(0xFF0A0C10)),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        if (!isAudioOnly) {
            // Video View
            Box(
                Modifier
                    .weight(1f)
                    .fillMaxWidth(),
                contentAlignment = Alignment.Center
            ) {
                AndroidView(
                    factory = { ctx ->
                        PlayerView(ctx).apply {
                            player = exoPlayer
                            useController = false // We provide unified TRUX controls
                        }
                    },
                    modifier = Modifier.fillMaxSize()
                )
            }
        } else {
            // Audio Visualizer / Emblem
            Box(
                Modifier
                    .weight(1f)
                    .fillMaxWidth(),
                contentAlignment = Alignment.Center
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Box(
                        modifier = Modifier
                            .size(100.dp)
                            .clip(CircleShape)
                            .background(Color(0xFF1E293B)),
                        contentAlignment = Alignment.Center
                    ) {
                        Icon(
                            imageVector = Icons.Default.MusicNote,
                            contentDescription = "Audio",
                            tint = Color(0xFF38BDF8),
                            modifier = Modifier.size(48.dp)
                        )
                    }
                    Spacer(Modifier.height(16.dp))
                    Text(
                        text = file.name,
                        color = Color(0xFFF1F5F9),
                        fontSize = 14.sp,
                        fontFamily = FontFamily.Monospace
                    )
                }
            }
        }

        // Unified Media Controls Bar
        Column(
            Modifier
                .fillMaxWidth()
                .background(Color(0xFF111620))
                .padding(horizontal = 20.dp, vertical = 12.dp)
        ) {
            // Progress Slider
            Slider(
                value = if (duration > 0) currentPos.toFloat() / duration.toFloat() else 0f,
                onValueChange = { fraction ->
                    val target = (fraction * duration).toLong()
                    currentPos = target
                    exoPlayer.seekTo(target)
                },
                colors = SliderDefaults.colors(
                    thumbColor = Color.White,
                    activeTrackColor = Color(0xFF38BDF8),
                    inactiveTrackColor = Color(0xFF334155)
                ),
                modifier = Modifier.fillMaxWidth()
            )

            // Time & Controls Row
            Row(
                Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = "${formatTime(currentPos)} / ${formatTime(duration)}",
                    color = Color(0xFF94A3B8),
                    fontSize = 11.sp,
                    fontFamily = FontFamily.Monospace
                )

                Row(verticalAlignment = Alignment.CenterVertically) {
                    // Rewind 10s
                    IconButton(
                        onClick = { exoPlayer.seekTo((exoPlayer.currentPosition - 10_000L).coerceAtLeast(0L)) }
                    ) {
                        Icon(
                            imageVector = Icons.Default.Replay10,
                            contentDescription = "Rewind 10s",
                            tint = Color(0xFFCBD5E1)
                        )
                    }

                    // Play/Pause
                    IconButton(
                        onClick = {
                            if (exoPlayer.isPlaying) {
                                exoPlayer.pause()
                            } else {
                                exoPlayer.play()
                            }
                        }
                    ) {
                        Icon(
                            imageVector = if (isPlaying) Icons.Default.Pause else Icons.Default.PlayArrow,
                            contentDescription = if (isPlaying) "Pause" else "Play",
                            tint = Color.White,
                            modifier = Modifier.size(32.dp)
                        )
                    }

                    // Forward 10s
                    IconButton(
                        onClick = { exoPlayer.seekTo((exoPlayer.currentPosition + 10_000L).coerceAtMost(duration)) }
                    ) {
                        Icon(
                            imageVector = Icons.Default.Forward10,
                            contentDescription = "Forward 10s",
                            tint = Color(0xFFCBD5E1)
                        )
                    }

                    // Mute / Unmute
                    IconButton(
                        onClick = {
                            isMuted = !isMuted
                            exoPlayer.volume = if (isMuted) 0f else 1f
                        }
                    ) {
                        Icon(
                            imageVector = if (isMuted) Icons.Default.VolumeOff else Icons.Default.VolumeUp,
                            contentDescription = "Mute Toggle",
                            tint = if (isMuted) Color(0xFFEF4444) else Color(0xFFCBD5E1)
                        )
                    }
                }
            }
        }
    }
}

private fun formatTime(millis: Long): String {
    val totalSeconds = (millis / 1000).coerceAtLeast(0)
    val minutes = totalSeconds / 60
    val seconds = totalSeconds % 60
    return String.format(Locale.US, "%02d:%02d", minutes, seconds)
}
