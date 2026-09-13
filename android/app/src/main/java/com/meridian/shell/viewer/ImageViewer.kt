package com.meridian.shell.viewer

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

@Composable
fun ImageViewer(file: File) {
    var bitmap by remember { mutableStateOf<Bitmap?>(null) }
    var imageWidth by remember { mutableIntStateOf(0) }
    var imageHeight by remember { mutableIntStateOf(0) }
    var scale by remember { mutableFloatStateOf(1f) }
    var offsetX by remember { mutableFloatStateOf(0f) }
    var offsetY by remember { mutableFloatStateOf(0f) }
    var errorMsg by remember { mutableStateOf<String?>(null) }
    var isLoading by remember { mutableStateOf(true) }

    LaunchedEffect(file) {
        isLoading = true
        withContext(Dispatchers.IO) {
            try {
                // First decode bounds only
                val options = BitmapFactory.Options().apply { inJustDecodeBounds = true }
                BitmapFactory.decodeFile(file.absolutePath, options)
                imageWidth = options.outWidth
                imageHeight = options.outHeight

                // Calculate sample size to fit comfortably in memory
                var sampleSize = 1
                val maxDim = 4096
                while (options.outWidth / sampleSize > maxDim || options.outHeight / sampleSize > maxDim) {
                    sampleSize *= 2
                }

                val decodeOptions = BitmapFactory.Options().apply {
                    inSampleSize = sampleSize
                    inPreferredConfig = Bitmap.Config.ARGB_8888
                }
                val decoded = BitmapFactory.decodeFile(file.absolutePath, decodeOptions)
                withContext(Dispatchers.Main) {
                    bitmap = decoded
                    isLoading = false
                    if (decoded == null) {
                        errorMsg = "Unable to decode image format."
                    }
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    errorMsg = "Failed to load image: ${e.message}"
                    isLoading = false
                }
            }
        }
    }

    Box(
        Modifier
            .fillMaxSize()
            .background(Color(0xFF0D1117)),
        contentAlignment = Alignment.Center
    ) {
        if (bitmap != null) {
            Image(
                bitmap = bitmap!!.asImageBitmap(),
                contentDescription = file.name,
                modifier = Modifier
                    .fillMaxSize()
                    .graphicsLayer(
                        scaleX = scale,
                        scaleY = scale,
                        translationX = offsetX,
                        translationY = offsetY
                    )
                    .pointerInput(Unit) {
                        detectTapGestures(
                            onDoubleTap = {
                                if (scale > 1.2f) {
                                    scale = 1f
                                    offsetX = 0f
                                    offsetY = 0f
                                } else {
                                    scale = 2.5f
                                }
                            }
                        )
                    }
                    .pointerInput(Unit) {
                        detectTransformGestures { _, pan, zoom, _ ->
                            scale = (scale * zoom).coerceIn(0.5f, 6f)
                            offsetX += pan.x
                            offsetY += pan.y
                        }
                    }
            )
        }

        if (isLoading) {
            CircularProgressIndicator(
                color = Color.White,
                modifier = Modifier.size(36.dp),
                strokeWidth = 2.dp
            )
        }

        if (errorMsg != null) {
            Text(
                text = errorMsg!!,
                color = Color(0xFFF87171),
                fontSize = 13.sp,
                fontFamily = FontFamily.Monospace
            )
        }

        // Info pill at bottom
        if (imageWidth > 0 && imageHeight > 0) {
            Box(
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .padding(bottom = 20.dp)
                    .clip(RoundedCornerShape(20.dp))
                    .background(Color.Black.copy(alpha = 0.6f))
                    .padding(horizontal = 14.dp, vertical = 6.dp)
            ) {
                Text(
                    text = "${imageWidth} × ${imageHeight} • ${(scale * 100).toInt()}%",
                    color = Color(0xFF94A3B8),
                    fontSize = 11.sp,
                    fontFamily = FontFamily.Monospace
                )
            }
        }
    }
}
