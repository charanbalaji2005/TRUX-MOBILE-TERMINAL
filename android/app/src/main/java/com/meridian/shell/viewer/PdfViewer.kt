package com.meridian.shell.viewer

import android.graphics.Bitmap
import android.graphics.pdf.PdfRenderer
import android.os.ParcelFileDescriptor
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectTransformGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material.icons.filled.Add
import androidx.compose.material3.*
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
fun PdfViewer(file: File) {
    var pageCount by remember { mutableIntStateOf(0) }
    var currentPageIndex by remember { mutableIntStateOf(0) }
    var currentBitmap by remember { mutableStateOf<Bitmap?>(null) }
    var scale by remember { mutableFloatStateOf(1f) }
    var offsetX by remember { mutableFloatStateOf(0f) }
    var offsetY by remember { mutableFloatStateOf(0f) }
    var errorMsg by remember { mutableStateOf<String?>(null) }
    var isLoading by remember { mutableStateOf(true) }

    var pfd by remember { mutableStateOf<ParcelFileDescriptor?>(null) }
    var renderer by remember { mutableStateOf<PdfRenderer?>(null) }

    // Initialize renderer
    DisposableEffect(file) {
        try {
            val descriptor = ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY)
            val pdfRenderer = PdfRenderer(descriptor)
            pfd = descriptor
            renderer = pdfRenderer
            pageCount = pdfRenderer.pageCount
        } catch (e: Exception) {
            errorMsg = "Unable to open PDF: ${e.message}"
        }

        onDispose {
            try {
                renderer?.close()
                pfd?.close()
            } catch (_: Exception) {}
        }
    }

    // Render active page
    LaunchedEffect(renderer, currentPageIndex, scale) {
        val r = renderer ?: return@LaunchedEffect
        if (pageCount <= 0) return@LaunchedEffect
        isLoading = true
        withContext(Dispatchers.IO) {
            try {
                val page = r.openPage(currentPageIndex)
                val targetWidth = (page.width * 2).coerceIn(400, 2048)
                val targetHeight = (page.height * 2).coerceIn(400, 2048)
                val bitmap = Bitmap.createBitmap(targetWidth, targetHeight, Bitmap.Config.ARGB_8888)
                bitmap.eraseColor(android.graphics.Color.WHITE)
                page.render(bitmap, null, null, PdfRenderer.Page.RENDER_MODE_FOR_DISPLAY)
                page.close()
                withContext(Dispatchers.Main) {
                    currentBitmap = bitmap
                    isLoading = false
                }
            } catch (e: Exception) {
                withContext(Dispatchers.Main) {
                    errorMsg = "Error rendering page: ${e.message}"
                    isLoading = false
                }
            }
        }
    }

    if (errorMsg != null) {
        Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            Text(errorMsg!!, color = Color(0xFFF87171), fontFamily = FontFamily.Monospace, fontSize = 13.sp)
        }
        return
    }

    Column(
        Modifier
            .fillMaxSize()
            .background(Color(0xFF0F1115))
    ) {
        // PDF Canvas with gestures
        Box(
            Modifier
                .weight(1f)
                .fillMaxWidth()
                .pointerInput(Unit) {
                    detectTransformGestures { _, pan, zoom, _ ->
                        scale = (scale * zoom).coerceIn(0.8f, 5f)
                        offsetX += pan.x
                        offsetY += pan.y
                    }
                },
            contentAlignment = Alignment.Center
        ) {
            if (currentBitmap != null) {
                Image(
                    bitmap = currentBitmap!!.asImageBitmap(),
                    contentDescription = "PDF Page ${currentPageIndex + 1}",
                    modifier = Modifier
                        .fillMaxSize()
                        .graphicsLayer(
                            scaleX = scale,
                            scaleY = scale,
                            translationX = offsetX,
                            translationY = offsetY
                        )
                )
            }

            if (isLoading) {
                CircularProgressIndicator(
                    color = Color.White,
                    modifier = Modifier.size(32.dp),
                    strokeWidth = 2.dp
                )
            }
        }

        // Bottom page controls bar
        Row(
            Modifier
                .fillMaxWidth()
                .background(Color(0xFF161B22))
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            // Page navigation
            Row(verticalAlignment = Alignment.CenterVertically) {
                IconButton(
                    onClick = {
                        if (currentPageIndex > 0) {
                            currentPageIndex--
                            offsetX = 0f
                            offsetY = 0f
                        }
                    },
                    enabled = currentPageIndex > 0
                ) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = "Previous Page",
                        tint = if (currentPageIndex > 0) Color.White else Color(0xFF475569)
                    )
                }

                Text(
                    text = "${currentPageIndex + 1} / $pageCount",
                    color = Color(0xFFE2E8F0),
                    fontSize = 12.sp,
                    fontFamily = FontFamily.Monospace,
                    modifier = Modifier.padding(horizontal = 8.dp)
                )

                IconButton(
                    onClick = {
                        if (currentPageIndex < pageCount - 1) {
                            currentPageIndex++
                            offsetX = 0f
                            offsetY = 0f
                        }
                    },
                    enabled = currentPageIndex < pageCount - 1
                ) {
                    Icon(
                        imageVector = Icons.AutoMirrored.Filled.ArrowForward,
                        contentDescription = "Next Page",
                        tint = if (currentPageIndex < pageCount - 1) Color.White else Color(0xFF475569)
                    )
                }
            }

            // Zoom controls
            Row(verticalAlignment = Alignment.CenterVertically) {
                Button(
                    onClick = { scale = (scale - 0.25f).coerceAtLeast(0.8f) },
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF21262D)),
                    contentPadding = PaddingValues(horizontal = 10.dp, vertical = 4.dp),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.height(30.dp)
                ) {
                    Text("−", color = Color.White, fontSize = 14.sp)
                }

                Spacer(Modifier.width(6.dp))

                Box(
                    modifier = Modifier
                        .clip(RoundedCornerShape(6.dp))
                        .background(Color(0xFF21262D))
                        .padding(horizontal = 8.dp, vertical = 4.dp)
                ) {
                    Text(
                        "${(scale * 100).toInt()}%",
                        color = Color(0xFF94A3B8),
                        fontSize = 11.sp,
                        fontFamily = FontFamily.Monospace
                    )
                }

                Spacer(Modifier.width(6.dp))

                Button(
                    onClick = { scale = (scale + 0.25f).coerceAtMost(5f) },
                    colors = ButtonDefaults.buttonColors(containerColor = Color(0xFF21262D)),
                    contentPadding = PaddingValues(horizontal = 10.dp, vertical = 4.dp),
                    shape = RoundedCornerShape(6.dp),
                    modifier = Modifier.height(30.dp)
                ) {
                    Icon(
                        imageVector = Icons.Default.Add,
                        contentDescription = "Zoom In",
                        tint = Color.White,
                        modifier = Modifier.size(14.dp)
                    )
                }
            }
        }
    }
}
