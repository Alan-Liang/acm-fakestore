// build instruction: `qjsc -o code main.js && strip code`

import * as std from 'std'

const env = 'staging'
const debug = env === 'development' ? print : () => {}

const beforeExitHooks = []
/** @returns {never} */
const exit = (...args) => {
  if (args.length !== 0) throw new Error('Syntax error')
  for (const f of beforeExitHooks) f()
  std.exit(0)
}

const formatCent = cent => `${Math.floor(cent / 100)}.${String(cent % 100).padStart(2, '0')}`
const parseCent = str => {
  if (!/^(?:\d{1,10}\.\d{2}|\d{1,11}\.\d|\d{1,12}\.|\d{1,13})$/.test(str)) throw new Error('Bad cent format')
  if (str.startsWith('0') && !str.startsWith('0.') && str !== '0') throw new Error('No octal numerals')
  return Math.round(Number(str) * 100)
}
const setupOptions = (obj, options) => {
  for (const k in options) if (options.hasOwnProperty(k) && k !== '_id') obj[k] = options[k]
  if (options.hasOwnProperty('_id')) try { obj._id = options._id } catch (e) {}
}

class Model {
  constructor (options = {}) {
    this._ensureInitialized()
    setupOptions(this, options)
    if (!('_id' in this)) this._id = this.constructor._getId(this)
    this._oldId = this._id
  }
  _ensureInitialized () { this.constructor._ensureInitialized() }
  static _ensureInitialized () { if (!this._initialized) this._init() }
  static _getId () { return Math.random().toString().slice(2) }
  static get _filename () { return (env === 'development' || env === 'staging' ? 'data/' : '') + this.name.toLowerCase() + 's' }
  static _init () {
    if (this._initialized) throw new Error('_init() called twice!')
    this._data = {}
    const file = std.open(this._filename, 'r')
    if (file) {
      const lines = file.readAsString().split('\n').filter(Boolean).map(x => JSON.parse(x))
      file.close()
      for (const line of lines) {
        if (line._deleted) delete this._data[line._id]
        else this._data[line._id] = line
      }
      const wfile = std.open(this._filename, 'w')
      wfile.puts(Object.values(this._data).map(x => JSON.stringify(x)).join('\n'))
      wfile.puts('\n')
      wfile.close()
    }
    this._file = std.open(this._filename, 'a')
    this._initialized = true
    beforeExitHooks.push(() => this._file.close())
  }
  save () {
    if (this._deleted) throw new Error('saving a deleted object')
    this._sanityCheck()
    const data = { ...this, _id: this._id }
    delete data._oldId
    this.constructor._data[this._id] = data
    if (this._oldId !== this._id) {
      delete this.constructor._data[this._oldId]
      this._write(`{"_id":"${this._oldId}","_deleted":true}\n`)
    }
    this._oldId = this._id
    this._write(JSON.stringify(data) + '\n')
    return this
  }
  delete () {
    this._write(`{"_id":"${this._id}","_deleted":true}\n`)
    this._deleted = true
    delete this.constructor._data[this._id]
  }
  _write (x) { this.constructor._file.puts(x) }
  _sanityCheck () {}
  static findById (id) {
    this._ensureInitialized()
    if (id === undefined || id === null) return null
    // debug(JSON.stringify(this._data))
    return this._data[id] ? new this(this._data[id]) : null
  }
  // supports _checkPredicateValue('kw2', [ 'kw1', 'kw2', 'kw3' ])
  static _checkPredicateValue (predicate, object) {
    if (Array.isArray(object)) return object.includes(predicate)
    return predicate === object
  }
  static _checkPredicate (predicate, object) {
    for (const k in predicate) if (predicate.hasOwnProperty(k) && !this._checkPredicateValue(predicate[k], object[k])) return false
    return true
  }
  static findOne (predicate) {
    this._ensureInitialized()
    for (const data of Object.values(this._data)) if (this._checkPredicate(predicate, data)) return new this(data)
    return null
  }
  static findMany (predicate, limit = Infinity) {
    this._ensureInitialized()
    const results = []
    const keys = Object.keys(this._data)
    if (limit !== Infinity) keys.sort(this._order)
    for (const key of keys) {
      const data = this._data[key]
      if (this._checkPredicate(predicate, data)) results.push(new this(data))
      if (results.length >= limit) return results
    }
    return results
  }
}

class Account extends Model {
  static PRIVILEGE_ROOT = 7
  static PRIVILEGE_WORKER = 3
  static PRIVILEGE_CUSTOMER = 1
  static PRIVILEGE_ANONYMOUS = 0
  static privileges = null // to be initialized
  id = ''
  password = ''
  name = ''
  privilege = 0
  // this line is needed due to some runtime semantics in class fields unknown to me
  constructor (options = {}) { super(options); setupOptions(this, options) }
  _sanityCheck () {
    if ([ this.id, this.password ].some(x => !/^[0-9a-zA-Z_]{1,30}$/.test(x))) throw new Error('Invalid userid or password')
    if (!/^[\x21-\x7E]{0,30}$/.test(this.name)) throw new Error('Invalid name')
    if (!Account.privileges.includes(this.privilege)) throw new Error('Invalid privilege')
    if (this.privilege === Account.PRIVILEGE_ANONYMOUS) throw new Error('Invalid privilege')
  }
  get _id () { return this.id }
  // TODO: use argon2 to hash password
  verifyPassword (password) { return this.password === password }
  setPassword (password) { this.password = password }
}
Account.privileges = [ Account.PRIVILEGE_ROOT, Account.PRIVILEGE_WORKER, Account.PRIVILEGE_CUSTOMER, Account.PRIVILEGE_ANONYMOUS ]

const validIntegerIn = (max, value) => !(
  !Number.isFinite(value) ||
  !Number.isInteger(value) ||
  value > max ||
  value < 0
)

class Book extends Model {
  static KEYWORDS_MAX_LENGTH = 60
  isbn = ''
  name = ''
  author = ''
  keywords = []
  stock = 0
  priceCent = 0
  constructor (options = {}) { super(options); setupOptions(this, options) }
  get _id () { return this.isbn }
  _sanityCheck () {
    if (!/^[\x21-\x7E]{1,20}$/.test(this.isbn)) throw new Error('Invalid ISBN')
    if ([ this.name, this.author ].some(x => (!/^[\x21-\x7E]{0,60}$/.test(x) || /"/.test(x)))) throw new Error('Invalid name or author')
    const keywordsLength = this.keywords.join('|').length
    if (keywordsLength > Book.KEYWORDS_MAX_LENGTH) throw new Error('Keywords too long')
    if (new Set(this.keywords).size !== this.keywords.length) throw new Error('Duplicate keywords')
    if (this.stock < 0) throw new Error('Something went wrong in our logic...')
    if (this.keywords.some(kw => !/^[\x21-\x7E]+$/.test(kw))) throw new Error('Invalid keyword')
  }
  static ensureIsbn (isbn) {
    return Book.findById(isbn) ?? new Book({ isbn }).save()
  }
  printf () {
    print(`${this.isbn}\t${this.name}\t${this.author}\t${this.keywords.sort().join('|')}\t${formatCent(this.priceCent)}\t${this.stock}`)
  }
}

class TransactionId extends Model {
  id = 0
  constructor (options = {}) { super(options); setupOptions(this, options) }
  get _id () { return 'singleton' }
  static nextId () {
    const id = this._singleton.id++
    this._singleton.save()
    return id
  }
}

TransactionId._singleton = TransactionId.findById('singleton') ?? new TransactionId().save()

class Transaction extends Model {
  static MAX_QUANTITY = 2_147_483_647
  static MAX_QUERY_LIMIT = 2_147_483_647
  bookId = ''
  type = '' // buy / import
  quantity = 0
  amountCent = 0
  constructor (options = {}) {
    super(options)
    setupOptions(this, options)
    if (this.type === 'buy' && !('amountCent' in options)) this.amountCent = this.book.priceCent * this.quantity
  }
  get book () { return Book.findById(this.bookId) }
  get _id () { return this.id ?? (this.id = TransactionId.nextId()) }
  static _order (a, b) { return b - a }
  _sanityCheck () {
    if (!this.book) throw new Error('No such book')
    if (![ 'buy', 'import' ].includes(this.type)) throw new Error('Invalid tx type')
    if (!validIntegerIn(Transaction.MAX_QUANTITY, this.quantity)) throw new Error('Invalid quantity')
    if (this.amountCent >= Number.MAX_SAFE_INTEGER) std.exit(1)
  }
}

class Version extends Model {
  version = 1
  constructor (options = {}) { super(options); setupOptions(this, options) }
  get _id () { return 'singleton' }
}

; {
  const versionSingleton = Version.findById('singleton')
  if (!versionSingleton) {
    new Version().save()
    new Account({
      id: 'root',
      password: 'sjtu',
      privilege: Account.PRIVILEGE_ROOT,
    }).save()
  }
}

class Shell {
  /** @type {Account | null} */
  account = null
  constructor (account) {
    this.account = account
  }
  get isAnonymous () { return this.account === null }
  get privilege () { return this.account?.privilege ?? 0 }
  /** @type {Book | null} */
  cursor = null

  requestPrivilege (privilege) {
    if (this.privilege < privilege) throw new Error('Access denied')
  }
}

/** @type {Shell[]} */
const shellStack = [ new Shell() ]
const getCurrentShell = () => shellStack[shellStack.length - 1]
const requestPrivilege = privilege => getCurrentShell().requestPrivilege(privilege)
const numberFromString = str => {
  if (str === '0') return 0
  if (str[0] === '0') throw new Error('No octal numbers')
  if (!/^\d+$/.test(str)) throw new Error('Not a number')
  return Number(str)
}

const commands = {
  exit,
  quit: exit,
  su (id, password) {
    const acct = Account.findById(id)
    if (!acct) throw new Error('Wrong credentials')
    const createShell = () => { shellStack.push(new Shell(acct)) }
    if (!password) {
      requestPrivilege(acct.privilege + 1)
      return createShell()
    }
    if (!acct.verifyPassword(password)) throw new Error('Wrong credentials')
    createShell()
  },
  logout () {
    requestPrivilege(Account.PRIVILEGE_CUSTOMER)
    shellStack.pop()
  },
  register (id, password, name) {
    if (!name) throw new Error('Syntax error')
    if (Account.findById(id)) throw new Error('Account already exists')
    new Account({ id, password, name, privilege: Account.PRIVILEGE_CUSTOMER }).save()
  },
  passwd (id, prev, curr) {
    requestPrivilege(Account.PRIVILEGE_CUSTOMER)
    const acct = Account.findById(id)
    if (!acct) throw new Error('No such account')
    if (!prev) throw new Error('Syntax error')
    if (!curr) {
      requestPrivilege(Account.PRIVILEGE_ROOT)
      acct.setPassword(prev)
      acct.save()
      return
    }
    if (!acct.verifyPassword(prev)) throw new Error('Bad credentials')
    acct.setPassword(curr)
    acct.save()
  },
  useradd (id, password, privilege, name) {
    if (!name) throw new Error('Syntax error')
    requestPrivilege(Account.PRIVILEGE_WORKER)
    privilege = numberFromString(privilege)
    if (privilege >= getCurrentShell().privilege) throw new Error('Access Denied')
    if (Account.findById(id)) throw new Error('Account already exists')
    new Account({ id, password, privilege, name }).save()
  },
  delete (id) {
    requestPrivilege(Account.PRIVILEGE_ROOT)
    const acct = Account.findById(id)
    if (!acct) throw new Error('No such account')
    if (shellStack.some(shell => shell.account?.id === id)) throw new Error('Account already logged in')
    acct.delete()
  },
  show (what, limit) {
    if (!limit) limit = Infinity
    requestPrivilege(Account.PRIVILEGE_CUSTOMER)
    if (what === 'finance') {
      requestPrivilege(Account.PRIVILEGE_ROOT)
      if (limit === 0) return void print('')
      if (limit !== Infinity && limit > Transaction.MAX_QUERY_LIMIT) throw new Error('Limit too large')
      const txs = Transaction.findMany({}, limit)
      if (limit !== Infinity && txs.length < limit) throw new Error('Limit too large')
      const income = txs.filter(tx => tx.type === 'buy').map(tx => tx.amountCent).reduce((a, b) => a + b, 0)
      const expense = txs.filter(tx => tx.type === 'import').map(tx => tx.amountCent).reduce((a, b) => a + b, 0)
      print(`+ ${formatCent(income)} - ${formatCent(expense)}`)
      return
    }
    if (limit !== Infinity) throw new Error('Garbage in input')
    const printPredicate = predicate => {
      const books = Book.findMany(predicate)
      if (books.length === 0) return void print('')
      const keys = books.map(x => x.isbn).sort()
      for (const k of keys) Book.findById(k).printf()
    }
    if (!what) return void printPredicate({})
    const re = /^(?:-ISBN=(?<isbn>[\x21-\x7E]{1,20})|-name="(?<name>[\x21-\x7E]{1,60})"|-author="(?<author>[\x21-\x7E]{1,60})"|-keyword="(?<keyword>[\x21-\x7E]{1,60})")$/
    const match = what.match(re)?.groups
    if (!match) throw new Error('Syntax error')
    for (const what of [ 'isbn', 'name', 'author', 'keyword' ]) if (match[what]) {
      if (what !== 'isbn' && match[what].includes('"')) throw new Error('Syntax error')
      if (what === 'keyword') {
        if (match.keyword.includes('|')) throw new Error('Cannot specify multiple keywords')
        printPredicate({ keywords: match.keyword })
      } else if (what === 'isbn') {
        const book = Book.findById(match.isbn)
        book ? book.printf() : print('')
        return
      } else printPredicate({ [what]: match[what] })
    }
  },
  buy (isbn, qty) {
    requestPrivilege(Account.PRIVILEGE_CUSTOMER)
    qty = numberFromString(qty)
    if (!validIntegerIn(Transaction.MAX_QUANTITY, qty)) throw new Error('Invalid quantity')
    const book = Book.findById(isbn)
    if (!book) throw new Error('No such book')
    if (book.stock < qty) throw new Error('Out of stock')
    book.stock -= qty
    // TODO: atomity
    book.save()
    print(formatCent(new Transaction({ bookId: isbn, type: 'buy', quantity: qty }).save().amountCent))
  },
  select (isbn) {
    requestPrivilege(Account.PRIVILEGE_WORKER)
    if (!isbn) throw new Error('Syntax error')
    Book.ensureIsbn(isbn)
    getCurrentShell().cursor = isbn
  },
  modify (...updates) {
    requestPrivilege(Account.PRIVILEGE_WORKER)
    if (updates.length === 0) throw new Error('Nothing to update')
    const book = Book.findById(getCurrentShell().cursor)
    if (!book) throw new Error('No update target selected')
    const updateField = {
      isbn (isbn) {
        if (Book.findById(isbn)) throw new Error('Duplicate ISBN')
        book.isbn = isbn
      },
      name (name) {
        if (name.includes('"')) throw new Error('Invalid name')
        book.name = name
      },
      author (author) {
        if (author.includes('"')) throw new Error('Invalid author')
        book.author = author
      },
      keyword (kw) {
        const keywords = kw.split('|')
        if (new Set(keywords).size !== keywords.length) throw new Error('Duplicate keywords')
        book.keywords = keywords
      },
      price (price) {
        const priceCent = parseCent(price)
        book.priceCent = priceCent
      },
    }
    const re = /^(?:-ISBN=(?<isbn>[\x21-\x7E]{1,20})|-name="(?<name>[\x21-\x7E]{1,60})"|-author="(?<author>[\x21-\x7E]{1,60})"|-keyword="(?<keyword>[\x21-\x7E]{1,60})"|-price=(?<price>\d{1,10}\.\d{2}|\d{1,11}\.\d|\d{1,12}\.|\d{1,13}))$/
    const matched = []
    for (const update of updates) {
      const match = update.match(re)?.groups
      if (!match) throw new Error('Syntax error')
      for (const what of [ 'isbn', 'name', 'author', 'keyword', 'price' ]) if (match[what]) {
        updateField[what](match[what])
        if (matched.includes(what)) throw new Error('Duplicate update')
        matched.push(what)
      }
    }
    book.save()
    for (const sh of shellStack) if (sh.cursor === getCurrentShell().cursor) sh.cursor = book.isbn
  },
  import (qty, totalCost) {
    requestPrivilege(Account.PRIVILEGE_WORKER)
    const book = Book.findById(getCurrentShell().cursor)
    if (!book) throw new Error('No import target selected')
    totalCost = parseCent(totalCost)
    qty = numberFromString(qty)
    if (!validIntegerIn(Transaction.MAX_QUANTITY, qty)) throw new Error('Invalid quantity')
    book.stock += qty
    book.save()
    new Transaction({ bookId: book.isbn, type: 'import', quantity: qty, amountCent: totalCost }).save()
  },
  report (what) {
    print('Method not implemented')
  },
  log () {
    print('Method not implemented')
  },
}

const trimSpaces = str => {
  if (!str) return ''
  let i = -1
  while (str[++i] === ' ') continue
  let j = str.length
  while (str[--j] === ' ') continue
  return str.slice(i, j + 1)
}

if (env === 'development') Account._ensureInitialized()
const MAX_LINE_LENGTH = 1024
const vaargCommands = [ 'modify' ]
const processLine = rawLine => {
  if (rawLine.length > MAX_LINE_LENGTH) throw new Error('Line too long')
  const line = trimSpaces(rawLine)
  if (!/^[\x20-\x7E]*$/.test(line)) throw new Error('Invalid charset')
  if (line.length === 0) return
  const [ arg0, ...argv ] = line.split(/ +/)
  if (env === 'development') debug('+ (' + shellStack.slice(1).map(x => x.account?.id).join(' ') + ')>', arg0, ...argv)
  if (!commands.hasOwnProperty(arg0)) throw new Error('No such command')
  if (!vaargCommands.includes(arg0) && argv.length > commands[arg0].length) throw new Error('Garbage in command')
  commands[arg0](...argv)
}
const oneLine = () => {
  if (std['in'].eof()) exit()
  const rawLine = std['in'].getline()
  if (!rawLine) return
  rawLine.split('\r').filter(Boolean).forEach(processLine)
}

while (true) try { oneLine() } catch (e) { debug(e); print('Invalid') }

exit()
