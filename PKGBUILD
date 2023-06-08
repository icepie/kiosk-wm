pkgname=kiosk-wm
pkgver=1.0
pkgrel=1
pkgdesc="A simple window manager for kiosk systems"
arch=('x86_64')
url="https://example.com/kiosk-wm"
license=('MIT')
depends=('libx11' 'libxext' 'libxfixes')

source=("kiosk-wm-run.sh"
    "kiosk-wm.c"
    "makefile"
)

sha256sums=('SKIP'
    'SKIP'
    'SKIP'
)

build() {
    cd "$srcdir"
    make
}

package() {
    install -Dm755 kiosk-wm "$pkgdir/usr/bin/kiosk-wm"
    install -Dm755 kiosk-wm-run.sh "$pkgdir/usr/bin/kiosk-wm-run.sh"
}