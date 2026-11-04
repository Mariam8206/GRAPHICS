# Regenerates the English UI PNGs in this folder from labels.tsv (run with Windows PowerShell 5.1).
# Font: Dubai Bold (ships with Windows). Text is white on transparent so the game can tint it.
# Every asset of one size class gets the same height (shared baseline), so labels and digits line up.
$ErrorActionPreference = "Stop"
$here = Split-Path $MyInvocation.MyCommand.Path
Add-Type -AssemblyName PresentationCore, PresentationFramework, WindowsBase
$sizes = @{ xs = 11; s = 13; m = 15; n = 18; l = 22; xl = 30; xxl = 58; btn = 30 }   # em size in pixels
# Image height per class (CLASS_HEIGHT in text_labels.cpp). Every image of a class is padded to it, so labels and digits line up
$heights = @{ xs = 14; s = 17; m = 22; n = 23; l = 33; xl = 32; xxl = 83; btn = 43 }
$face = New-Object System.Windows.Media.Typeface (New-Object System.Windows.Media.FontFamily "Dubai"), ([System.Windows.FontStyles]::Normal), ([System.Windows.FontWeights]::Bold), ([System.Windows.FontStretches]::Normal)

$items = @()
foreach ($line in (Get-Content "$here\labels.tsv" -Encoding UTF8 | Select-Object -Skip 1)) {
    if ($line.Trim() -eq "") { continue }
    $p = $line -split "`t"; $items += ,@($p[0], $p[1], $p[2])                # id, class, text
}
# digits per class: 0..9, slash, plus, decimal point
$glyphs = @{ "0"="0"; "1"="1"; "2"="2"; "3"="3"; "4"="4"; "5"="5"; "6"="6"; "7"="7"; "8"="8"; "9"="9"; "slash"="/"; "plus"="+"; "dot"="." }
foreach ($c in $sizes.Keys) { foreach ($g in $glyphs.Keys) { $items += ,@("$($c)_digit_$g", $c, [string]$glyphs[$g]) } }

function Render($text, $em) {
    $ft = New-Object System.Windows.Media.FormattedText $text, ([System.Globalization.CultureInfo]::GetCultureInfo("en-US")), ([System.Windows.FlowDirection]::LeftToRight), $face, $em, ([System.Windows.Media.Brushes]::White), 1.0
    $w = [int][math]::Ceiling($ft.WidthIncludingTrailingWhitespace) + 8; $h = [int][math]::Ceiling($ft.Height) + 8
    $dv = New-Object System.Windows.Media.DrawingVisual
    [System.Windows.Media.TextOptions]::SetTextFormattingMode($dv, [System.Windows.Media.TextFormattingMode]::Display)
    $dc = $dv.RenderOpen()
    $dc.DrawText($ft, (New-Object System.Windows.Point 4, 4)); $dc.Close()
    $rtb = New-Object System.Windows.Media.Imaging.RenderTargetBitmap $w, $h, 96, 96, ([System.Windows.Media.PixelFormats]::Pbgra32)
    $rtb.Render($dv)
    $px = New-Object byte[] ($w * $h * 4); $rtb.CopyPixels($px, $w * 4, 0)
    return @{ w = $w; h = $h; px = $px }
}

# pass 1: render and measure ink bounds; pass 2: crop (horizontal = own ink, vertical = class-wide ink box)
$rendered = @{}; $classTop = @{}; $classBottom = @{}
foreach ($it in $items) {
    $r = Render $it[2] $sizes[$it[1]]
    $minX = $r.w; $maxX = -1; $minY = $r.h; $maxY = -1
    for ($y = 0; $y -lt $r.h; $y++) { for ($x = 0; $x -lt $r.w; $x++) { if ($r.px[($y * $r.w + $x) * 4 + 3] -gt 8) {
        if ($x -lt $minX) { $minX = $x }; if ($x -gt $maxX) { $maxX = $x }; if ($y -lt $minY) { $minY = $y }; if ($y -gt $maxY) { $maxY = $y } } } }
    $r.minX = $minX; $r.maxX = $maxX; $rendered[$it[0]] = $r
    $c = $it[1]
    if (-not $classTop.ContainsKey($c) -or $minY -lt $classTop[$c]) { $classTop[$c] = $minY }
    if (-not $classBottom.ContainsKey($c) -or $maxY -gt $classBottom[$c]) { $classBottom[$c] = $maxY }
}
foreach ($it in $items) {
    $r = $rendered[$it[0]]; $c = $it[1]
    $x0 = [math]::Max(0, $r.minX - 1); $x1 = [math]::Min($r.w - 1, $r.maxX + 1)
    if ($it[2] -eq " " -or $r.maxX -lt 0) { $x0 = 0; $x1 = 3 }
    $y0 = [math]::Max(0, $classTop[$c] - 1); $y1 = [math]::Min($r.h - 1, $classBottom[$c] + 1)
    $cw = $x1 - $x0 + 1; $ch = $heights[$c]
    $pad = [math]::Floor(($ch - ($y1 - $y0 + 1)) / 2)                          # ink centered in the class height
    $out = New-Object byte[] ($cw * $ch * 4)
    for ($y = 0; $y -lt $ch; $y++) { for ($x = 0; $x -lt $cw; $x++) {
        $sy = $y - $pad + $y0
        $a = if ($sy -ge $y0 -and $sy -le $y1) { $r.px[($sy * $r.w + ($x + $x0)) * 4 + 3] } else { 0 }
        $o = ($y * $cw + $x) * 4; $out[$o] = 255; $out[$o + 1] = 255; $out[$o + 2] = 255; $out[$o + 3] = $a } }   # straight (not premultiplied) white
    $bmp = [System.Windows.Media.Imaging.BitmapSource]::Create($cw, $ch, 96, 96, [System.Windows.Media.PixelFormats]::Bgra32, $null, $out, $cw * 4)
    $enc = New-Object System.Windows.Media.Imaging.PngBitmapEncoder
    $enc.Frames.Add([System.Windows.Media.Imaging.BitmapFrame]::Create($bmp))
    $fs = [System.IO.File]::Create("$here\$($it[0]).png"); $enc.Save($fs); $fs.Close()
}
"generated $($items.Count) PNGs"
