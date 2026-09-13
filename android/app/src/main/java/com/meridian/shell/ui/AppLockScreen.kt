package com.meridian.shell.ui

import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Lock
import androidx.compose.material3.Icon
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.meridian.shell.R

/**
 * Minimalist full-screen TRUX lock screen.
 * Completely obscures terminal output, sessions, and working directories until authenticated.
 */
@Composable
fun AppLockScreen(
    onUnlockRequest: () -> Unit,
    errorMessage: String? = null
) {
    // Automatically trigger unlock prompt upon screen appearance
    LaunchedEffect(Unit) {
        onUnlockRequest()
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color(0xFF000000))
            .systemBarsPadding(),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center,
            modifier = Modifier.padding(32.dp)
        ) {
            // Metallic TRUX Emblem
            Image(
                painter = painterResource(id = R.drawable.trux_logo),
                contentDescription = "TRUX Emblem",
                modifier = Modifier
                    .size(110.dp)
                    .clip(RoundedCornerShape(26.dp))
            )

            Spacer(Modifier.height(28.dp))

            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.Center
            ) {
                Icon(
                    imageVector = Icons.Default.Lock,
                    contentDescription = "App Lock",
                    tint = Color(0xFFE2E8F0),
                    modifier = Modifier.size(20.dp)
                )
                Spacer(Modifier.width(8.dp))
                Text(
                    text = "TRUX is locked",
                    color = Color.White,
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Bold,
                    fontFamily = FontFamily.Monospace,
                    letterSpacing = 1.sp
                )
            }

            Spacer(Modifier.height(8.dp))

            Text(
                text = "Authenticate to continue",
                color = Color(0xFF94A3B8),
                fontSize = 12.sp,
                fontFamily = FontFamily.Monospace,
                letterSpacing = 0.5.sp
            )

            Spacer(Modifier.height(32.dp))

            // Unlock Button
            Box(
                modifier = Modifier
                    .clip(RoundedCornerShape(10.dp))
                    .background(Color(0xFF1E293B))
                    .border(1.dp, Color(0xFF334155), RoundedCornerShape(10.dp))
                    .clickable { onUnlockRequest() }
                    .padding(horizontal = 36.dp, vertical = 12.dp),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = "Unlock",
                    color = Color(0xFFF8FAFC),
                    fontSize = 14.sp,
                    fontWeight = FontWeight.SemiBold,
                    fontFamily = FontFamily.Monospace,
                    letterSpacing = 1.sp
                )
            }

            if (!errorMessage.isNullOrBlank()) {
                Spacer(Modifier.height(20.dp))
                Text(
                    text = errorMessage,
                    color = Color(0xFFF87171),
                    fontSize = 11.sp,
                    fontFamily = FontFamily.Monospace,
                    modifier = Modifier.padding(horizontal = 16.dp)
                )
            }
        }

        // Bottom security indicator
        Text(
            text = "PROTECTED BY ANDROID BIOMETRICS",
            color = Color(0xFF475569),
            fontSize = 9.sp,
            fontFamily = FontFamily.Monospace,
            letterSpacing = 1.5.sp,
            modifier = Modifier
                .align(Alignment.BottomCenter)
                .padding(bottom = 24.dp)
        )
    }
}
