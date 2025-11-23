pkgname=cafetch
pkgver=1.0.0
pkgrel=1
pkgdesc="A fast system information fetcher written in Go"
arch=('x86_64')
url="https://github.com/catowner-sudo/cafetch"
license=('MIT')
depends=('go')
makedepends=('git')
source=("git+https://github.com/catowner-sudo/cafetch.git")
md5sums=('SKIP')

build() {
    cd "$srcdir/cafetch"
    go build -o cafetch
}

package() {
    cd "$srcdir/cafetch"
    install -Dm755 cafetch "$pkgdir/usr/bin/cafetch"
}
