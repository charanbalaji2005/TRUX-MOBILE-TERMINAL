package com.meridian.shell.ui

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.expandVertically
import androidx.compose.animation.shrinkVertically
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.meridian.shell.R

/**
 * TRUX Welcome Banner displayed at the top of the terminal screen.
 * Matches the official TRUX visual branding:
 * - Glowing metallic TRUX emblem
 * - WELCOME TO TRUX ANDROID TERMINAL (BUILD > EXPLORE > BEYOND)
 * - 6 Core Capability Highlights (Linux Environment, Package Ecosystem, AI Assistant, App Lock, File Viewer, High Performance)
 * - Terminal quick hints
 * - Collapsible toggle to maximize terminal screen space
 */
@Composable
fun TruxWelcomeBanner(
    modifier: Modifier = Modifier,
    initialExpanded: Boolean = true
) {
    var isExpanded by remember { mutableStateOf(initialExpanded) }

    Column(
        modifier = modifier
            .fillMaxWidth()
            .background(Color(0xFF000000))
            .padding(horizontal = 12.dp, vertical = 6.dp)
    ) {
        // Top row: Emblem + Title + Collapse Toggle
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 4.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            // Metallic TRUX Emblem with circular halo glow
            Box(
                modifier = Modifier
                    .size(if (isExpanded) 56.dp else 36.dp)
                    .clip(CircleShape)
                    .background(Color(0xFF0F1218))
                    .border(
                        width = 1.2.dp,
                        brush = Brush.radialGradient(
                            colors = listOf(Color(0xFFFFFFFF), Color(0xFF64748B), Color(0xFF1E293B))
                        ),
                        shape = CircleShape
                    ),
                contentAlignment = Alignment.Center
            ) {
                Image(
                    painter = painterResource(id = R.drawable.trux_logo),
                    contentDescription = "TRUX Logo",
                    modifier = Modifier
                        .fillMaxSize()
                        .clip(CircleShape)
                )
            }

            Spacer(Modifier.width(12.dp))

            // Title branding
            Column(Modifier.weight(1f)) {
                if (isExpanded) {
                    Text(
                        text = "WELCOME TO",
                        color = Color(0xFF94A3B8),
                        fontSize = 9.sp,
                        fontFamily = FontFamily.Monospace,
                        fontWeight = FontWeight.SemiBold,
                        letterSpacing = 2.sp
                    )
                }
                Text(
                    text = "TRUX",
                    color = Color.White,
                    fontSize = if (isExpanded) 22.sp else 16.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Black,
                    letterSpacing = if (isExpanded) 5.sp else 3.sp
                )
                Text(
                    text = "ANDROID TERMINAL",
                    color = Color(0xFFCBD5E1),
                    fontSize = if (isExpanded) 9.5.sp else 9.sp,
                    fontFamily = FontFamily.Monospace,
                    fontWeight = FontWeight.Bold,
                    letterSpacing = 2.sp
                )
                if (isExpanded) {
                    Text(
                        text = "BUILD  >  EXPLORE  >  BEYOND",
                        color = Color(0xFF64748B),
                        fontSize = 7.5.sp,
                        fontFamily = FontFamily.Monospace,
                        fontWeight = FontWeight.Medium,
                        letterSpacing = 1.5.sp,
                        modifier = Modifier.padding(top = 1.dp)
                    )
                }
            }

            // Collapse / Expand toggle button
            IconButton(
                onClick = { isExpanded = !isExpanded },
                modifier = Modifier.size(32.dp)
            ) {
                Icon(
                    imageVector = if (isExpanded) Icons.Default.KeyboardArrowUp else Icons.Default.KeyboardArrowDown,
                    contentDescription = if (isExpanded) "Collapse banner" else "Expand banner",
                    tint = Color(0xFF94A3B8),
                    modifier = Modifier.size(20.dp)
                )
            }
        }

        // Expanded features and hints
        AnimatedVisibility(
            visible = isExpanded,
            enter = expandVertically(),
            exit = shrinkVertically()
        ) {
            Column(Modifier.fillMaxWidth()) {
                // Divider line
                HorizontalDivider(
                    color = Color(0xFF1E293B),
                    thickness = 0.8.dp,
                    modifier = Modifier.padding(vertical = 6.dp)
                )

                // 6 Core Features Row
                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(vertical = 4.dp),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.Top
                ) {
                    FeatureItem(
                        icon = null,
                        customSymbol = ">_",
                        title = "Powerful",
                        subtitle = "Linux Env",
                        modifier = Modifier.weight(1f)
                    )
                    FeatureDivider()
                    FeatureItem(
                        icon = Icons.Default.Inventory2,
                        title = "Full Package",
                        subtitle = "Ecosystem",
                        modifier = Modifier.weight(1f)
                    )
                    FeatureDivider()
                    FeatureItem(
                        icon = Icons.Default.SmartToy,
                        title = "Integrated",
                        subtitle = "AI Assistant",
                        modifier = Modifier.weight(1f)
                    )
                    FeatureDivider()
                    FeatureItem(
                        icon = Icons.Default.Lock,
                        title = "Secure",
                        subtitle = "App Lock",
                        modifier = Modifier.weight(1f)
                    )
                    FeatureDivider()
                    FeatureItem(
                        icon = Icons.Default.Image,
                        title = "Universal",
                        subtitle = "File Viewer",
                        modifier = Modifier.weight(1f)
                    )
                    FeatureDivider()
                    FeatureItem(
                        icon = Icons.Default.Bolt,
                        title = "Lightweight",
                        subtitle = "Performance",
                        modifier = Modifier.weight(1f)
                    )
                }

                // Divider line
                HorizontalDivider(
                    color = Color(0xFF1E293B),
                    thickness = 0.8.dp,
                    modifier = Modifier.padding(top = 6.dp, bottom = 4.dp)
                )

                // Command guide hints from Image 1
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 4.dp, vertical = 2.dp)
                ) {
                    CommandHint("Type 'help' to see available commands")
                    CommandHint("Type 'trux' to explore TRUX tools")
                    CommandHint("Type 'pkg' to manage packages")
                    CommandHint("Type 'trux ai' to use AI assistant")
                }
            }
        }
    }
}

@Composable
private fun FeatureItem(
    icon: ImageVector?,
    title: String,
    subtitle: String,
    customSymbol: String? = null,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier.padding(horizontal = 2.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Box(
            modifier = Modifier.size(20.dp),
            contentAlignment = Alignment.Center
        ) {
            if (customSymbol != null) {
                Text(
                    text = customSymbol,
                    color = Color.White,
                    fontSize = 13.sp,
                    fontWeight = FontWeight.Bold,
                    fontFamily = FontFamily.Monospace
                )
            } else if (icon != null) {
                Icon(
                    imageVector = icon,
                    contentDescription = null,
                    tint = Color.White,
                    modifier = Modifier.size(17.dp)
                )
            }
        }

        Spacer(Modifier.height(3.dp))

        Text(
            text = title,
            color = Color(0xFFCBD5E1),
            fontSize = 7.5.sp,
            fontFamily = FontFamily.Monospace,
            fontWeight = FontWeight.SemiBold,
            textAlign = TextAlign.Center,
            lineHeight = 9.sp,
            maxLines = 1
        )
        Text(
            text = subtitle,
            color = Color(0xFF64748B),
            fontSize = 7.sp,
            fontFamily = FontFamily.Monospace,
            textAlign = TextAlign.Center,
            lineHeight = 8.sp,
            maxLines = 1
        )
    }
}

@Composable
private fun FeatureDivider() {
    Box(
        modifier = Modifier
            .width(0.5.dp)
            .height(28.dp)
            .background(Color(0xFF1E293B))
    )
}

@Composable
private fun CommandHint(text: String) {
    Text(
        text = text,
        color = Color(0xFF94A3B8),
        fontSize = 10.sp,
        fontFamily = FontFamily.Monospace,
        lineHeight = 14.sp
    )
}
