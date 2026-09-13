package com.meridian.shell.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

/** Keys the Android IME does not provide but a terminal cannot live without. */
enum class ExtraKey(val label: String) {
    ESC("ESC"), SLASH("/"), DASH("-"), HOME("HOME"), UP("↑"), END("END"), PGUP("PGUP"),
    TAB("TAB"), CTRL("CTRL"), ALT("ALT"), LEFT("←"), DOWN("↓"), RIGHT("→"), PGDN("PGDN")
}

@Composable
fun ExtraKeysRow(
    ctrlLatched: Boolean,
    altLatched: Boolean,
    onKey: (ExtraKey) -> Unit,
    modifier: Modifier = Modifier
) {
    val top = listOf(ExtraKey.ESC, ExtraKey.SLASH, ExtraKey.DASH, ExtraKey.HOME,
                     ExtraKey.UP, ExtraKey.END, ExtraKey.PGUP)
    val bottom = listOf(ExtraKey.TAB, ExtraKey.CTRL, ExtraKey.ALT, ExtraKey.LEFT,
                        ExtraKey.DOWN, ExtraKey.RIGHT, ExtraKey.PGDN)

    Column(
        modifier
            .fillMaxWidth()
            .background(Color(0xFF0D0D0D))
            .padding(horizontal = 4.dp, vertical = 3.dp)
    ) {
        listOf(top, bottom).forEach { row ->
            Row(
                Modifier
                    .fillMaxWidth()
                    .height(38.dp)
                    .padding(vertical = 2.dp)
            ) {
                row.forEach { key ->
                    val latched = (key == ExtraKey.CTRL && ctrlLatched) ||
                                  (key == ExtraKey.ALT && altLatched)
                    Box(
                        Modifier
                            .weight(1f)
                            .fillMaxHeight()
                            .padding(horizontal = 2.dp)
                            .clip(RoundedCornerShape(6.dp))
                            .background(if (latched) Color(0xFF383838) else Color(0xFF1E1E1E))
                            .clickable { onKey(key) },
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            key.label,
                            color = if (latched) Color(0xFF7FD1FF) else Color(0xFFE0E0E0),
                            fontSize = 12.sp,
                            fontFamily = FontFamily.Monospace,
                            fontWeight = if (latched) FontWeight.Bold else FontWeight.Medium,
                            textAlign = TextAlign.Center
                        )
                    }
                }
            }
        }
    }
}

