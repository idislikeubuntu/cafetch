pkgname=cafetch
pkgver=1.0.0
pkgrel=1
pkgdesc="Written in go, a system informer as a CLI"
arch=('x86_64')
url="https://github.com/catowner-sudo/cafetch"
license=('GNU GPLv3')
depends=('go')
source=("https://github.com/catowner-sudo/cafetch.git")
md5sums=('SKIP')

build() {
    cd "$srcdir/cafetch"
    go build -o cafetch
}

package() {
    cd "$srcdir/cafetch"
    install -Dm755 cafetch "$pkgdir/usr/bin/cafetch"
}
