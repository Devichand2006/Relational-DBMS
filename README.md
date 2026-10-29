
# 🗄️ NITCBase — Mini Relational Database Management System

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-blue?style=for-the-badge&logo=cplusplus"/>
  <img src="https://img.shields.io/badge/Platform-Linux-orange?style=for-the-badge&logo=linux"/>
  <img src="https://img.shields.io/badge/Status-Completed-brightgreen?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Stages-12%2F12-blueviolet?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Institution-NIT%20Calicut-red?style=for-the-badge"/>
</p>

<p align="center">
  A fully functional mini RDBMS built from scratch in C++ — spanning disk simulation, buffer management, B+ Tree indexing, relational algebra, and an interactive SQL-like interface.
</p>

<p align="center">
  <a href="#-overview">Overview</a> •
  <a href="#-architecture">Architecture</a> •
  <a href="#-features">Features</a> •
  <a href="#-project-structure">Structure</a> •
  <a href="#-getting-started">Getting Started</a> •
  <a href="#-usage">Usage</a> •
  <a href="#-implementation-stages">Stages</a> •
  <a href="#-documentation">Docs</a> •
  <a href="#-contributing">Contributing</a>
</p>

---

## 📌 Overview

**NITCBase** is a mini Relational Database Management System built entirely from scratch in C++ as part of the Database Lab course at **NIT Calicut**. It faithfully replicates the internals of a real-world database engine — from raw 2048-byte block I/O on a simulated 16MB disk all the way up to relational algebra query execution — across **12 fully completed stages**.

This is not a wrapper around an existing DB engine. Every subsystem — disk I/O, buffer management, metadata caching, B+ Tree indexing, and query processing — is independently implemented from the ground up, making it a deep systems-level project in C++.

### ✨ At a Glance

| Component | Details |
|---|---|
| 🖴 Disk Simulation | 16MB virtual disk, 8192 blocks × 2048 bytes each |
| ⚡ Buffer Manager | 32-block pool with LRU replacement & dirty-bit writeback |
| 🗂️ Metadata Cache | Relation & Attribute Cache for up to 12 open relations |
| 🌲 B+ Tree Index | Dynamic insert, delete & search with node splitting/merging |
| 🔍 Query Engine | Select, project, insert, and equi-join |
| 💬 CLI | Interactive SQL-like interface with batch script support |
| 🛠️ XFS Interface | Separate external filesystem tool for disk inspection |

---
## 🏛 Architecture

```
                        ┌──────────┐
                        │   User   │
                        └────┬─────┘
                             │
              ┌──────────────▼───────────────┐
              │  Frontend User Interface     │  ← FrontendInterface.cpp
              │  (Parses SQL-like commands)  │
              └──────────────┬───────────────┘
                             │  Translation of SQL-like commands to class methods
              ┌──────────────▼───────────────┐
              │  Frontend Programming        │  ← Frontend.cpp
              │  Interface                   │  ← Methods corresponding to each query
              └──────────────┬───────────────┘
                             │  Translation to lower layer function calls
           ┌─────────────────┴──────────────────┐
           │                                    │
  ┌────────▼────────┐                 ┌─────────▼────────┐
  │  Algebra Layer  │                 │   Schema Layer   │
  └────────┬────────┘                 └────────┬─────────┘
           └──────────────┬─────────────────────┘
                          │
           ┌──────────────▼───────────┐     ┌──────────────────────────────────┐
           │   Block Access Layer     │◄───►│          Cache Layer             │
           └──────────────┬───────────┘     │  ┌──────────────┐  ┌──────────┐ │
                          │                 │  │RelCacheTable │  │AttrCache │ │
  ┌───────────────────────▼──────────┐      │  └──────┬───────┘  └────┬─────┘ │
  │         B+ Tree Layer            │      │  Friend └──────┬─────────┘Friend │
  └───────────────────────┬──────────┘      │         ┌──────▼───────┐         │
                          │                 │         │ OpenRelTable │         │
                          │                 └─────────┴──────────────┴─────────┘
                          │                                   │
                          └──────────────┬────────────────────┘
                                         │
                    ┌────────────────────▼────────────────────┐
                    │              Buffer Layer                │
                    │  ┌──────────────┐     ┌──────────────┐  │
                    │  │ StaticBuffer │◄────│  BlockBuffer │  │
                    │  └──────────────┘     └──────────────┘  │
                    └────────────────────┬────────────────────┘
                                         │
                    ┌────────────────────▼────────────────────┐
                    │            Physical Layer                │
                    │   ┌────────────┐                        │
                    │   │ Disk Class │  readBlock()           │
                    │   │            │  writeBlock()          │
                    │   └─────┬──────┘                        │
                    │      ┌──▼───┐                           │
                    │      │ DISK │                           │
                    │      └──────┘                           │
                    └─────────────────────────────────────────┘
```

> Each layer communicates **only** with the layer directly below it. The Cache Layer sits alongside Block Access and B+ Tree layers, with `RelCacheTable`, `AttrCacheTable`, and `OpenRelTable` connected via friend relationships. The Buffer Layer's `StaticBuffer` and `BlockBuffer` are also connected via a friend relationship.

> 📐 For a more detailed design diagram of the architecture, refer to the [NITCBase Design Diagram](https://nitcbase.github.io/docs/Design/DesignDiagram).
---

## 🔧 Features

### 💾 Disk & Storage
- **Simulated 16MB disk** stored as a binary file, split into **8192 blocks of 2048 bytes** each
- Block types: Record blocks, Internal Index blocks, Leaf Index blocks, and a Block Allocation Map
- A **disk run-copy** mechanism preserves disk state across batch script runs for safe testing

### ⚡ Buffer Manager
- **32-block in-memory buffer pool** (`StaticBuffer`) shared across all layers
- **LRU replacement policy** using timestamps — the least recently used block is evicted when the pool is full
- **Dirty-bit writeback** — modified blocks are flushed to disk only on eviction or system exit, minimising unnecessary disk writes
- Block Allocation Map kept in memory throughout a session for fast free-block lookup

### 🗂️ Cache Layer
- **Relation Cache** (`RelCacheTable`) — stores relation metadata (name, #attributes, #records, first/last block, slots/block) for up to 12 open relations
- **Attribute Cache** (`AttrCacheTable`) — stores per-attribute metadata (name, type, offset, B+ Tree root block) with search index tracking for index scans
- **Open Relation Table** (`OpenRelTable`) — manages the lifecycle of open relations and initialises RELATIONCAT and ATTRIBUTECAT automatically on startup

### 🌲 B+ Tree Indexing
- Full B+ Tree implementation supporting **insert, delete, and search**
- **Leaf nodes** hold up to 63 index entries (`(attribute value, block number, slot number)`-32 bytes each)
- **internal nodes** hold up to 100 keys (16 bytes each) with 101 child pointers (4 bytes each)
- Node splits on overflow (leaf at index 31, internal at index 50) and merges/borrows on underflow
- Index entries store `(attribute value, block number, slot number)` for direct record retrieval
- Leaf nodes are **linked as a chain** for efficient range scans (doubly linked list)

### 🔍 Query Engine

**Schema (DDL):**

| Command | Function |
|---|---|
| `CREATE TABLE` | `Schema::createRel()` |
| `DROP TABLE` | `Schema::deleteRel()` |
| `OPEN / CLOSE TABLE` | `Schema::openRel()` / `Schema::closeRel()` |
| `ALTER TABLE RENAME / COLUMN` | `Schema::renameRel()` / `Schema::renameAttr()` |
| `CREATE INDEX` | `Schema::createIndex()` |
| `DROP INDEX` | `Schema::dropIndex()` |

**Algebra (DML):**

| Command | Function |
|---|---|
| `INSERT` | `Algebra::insert()` |
| `SELECT` | `Algebra::select()` (full & partial) |
| `JOIN` | `Algebra::join()` (equi-join) |

Supported condition operators: `=`, `<=`, `<`, `>=`, `>`, `!=`

### 💬 Frontend Interface
- **Interactive CLI** powered by `libreadline` with command history
- **Batch script execution** — load and run `.txt` script files with `INSERT INTO ... VALUES FROM <file>.csv`
- Built-in `echo` support in batch files for annotated script output
- Input CSVs, output CSVs, and batch scripts kept in organised `Files/` directories

### 🛠️ XFS Interface
A separate **external filesystem tool** (`XFS_Interface/xfs-interface`) for inspecting and manipulating the raw disk image directly — useful for verifying catalog entries, examining block contents, and testing disk-level structures independently of the main engine.

---

## 📁 Project Structure

```
NITCBase-RDBMS/
│
├── mynitcbase/                        # Main RDBMS engine
│   ├── main.cpp                       # Entry point — initialises Disk, Buffer, Cache
│   ├── Makefile                       # Supports normal and debug (mode=debug) builds
│   │
│   ├── define/
│   │   ├── constants.h                # All system constants (block size, buffer capacity, etc.)
│   │   └── id.h                       # RecId, IndexId structs
│   │
│   ├── Disk_Class/
│   │   └── Disk.h / Disk.cpp          # Raw block read/write on the simulated 16MB disk
│   │
│   ├── Buffer/
│   │   ├── StaticBuffer.h/.cpp        # 32-block buffer pool, LRU replacement, Block Alloc Map
│   │   └── BlockBuffer.h/.cpp         # Block abstraction — RecBuffer, IndBuffer (571 lines)
│   │
│   ├── Cache/
│   │   ├── RelCacheTable.h/.cpp       # Relation metadata cache
│   │   ├── AttrCacheTable.h/.cpp      # Attribute metadata + search index cache
│   │   └── OpenRelTable.h/.cpp        # Open relation lifecycle manager (446 lines)
│   │
│   ├── BPlusTree/
│   │   └── BPlusTree.h/.cpp           # Full B+ Tree: search, insert, delete (941 lines)
│   │
│   ├── BlockAccess/
│   │   └── BlockAccess.h/.cpp         # Record scan, insert, delete, search (846 lines)
│   │
│   ├── Schema/
│   │   └── Schema.h/.cpp              # DDL — create, drop, open, close, rename, index (277 lines)
│   │
│   ├── Algebra/
│   │   └── Algebra.h/.cpp             # DML — select, project, insert, join (560 lines)
│   │
│   ├── Frontend/
│   │   └── Frontend.h/.cpp            # Command dispatcher to Schema/Algebra
│   │
│   └── FrontendInterface/
│       ├── FrontendInterface.h/.cpp   # CLI parser, readline integration (607 lines)
│       └── RegexHandler.h             # Command pattern matching
│
├── Disk/
│   ├── disk                           # Simulated 16MB disk (binary)
│   └── disk_run_copy                  # Backup copy for batch script runs
│
├── Files/
│   ├── Input_Files/                   # CSV files for bulk INSERT (students, products, etc.)
│   ├── Output_Files/                  # Query output CSVs, catalog dumps
│   └── Batch_Execution_Files/         # .txt batch scripts for stage-wise testing
│
└── XFS_Interface/                     # External filesystem inspection tool
    ├── xfs-interface                  # Compiled binary
    └── *.cpp / *.h                    # Source for disk-level debugging tool
```

---

## 🛠️ Prerequisites

Tested on **Ubuntu 20.04 LTS**. Install all dependencies with:

```bash
sudo apt-get install build-essential libreadline-dev git
```

| Tool | Purpose |
|---|---|
| `g++` | C++ compiler |
| `make` | Build automation |
| `libreadline-dev` | Interactive CLI with command history |
| `git` | Version control |

---

## 🚀 Getting Started

### 1. Clone the Repository

```bash
git clone https://github.com/Krishna-2105/NITCBase-RDBMS
cd NITCBase-RDBMS/mynitcbase
```

### 2. Build the Project

```bash
# Standard build
make

# Debug build (with -g flag, outputs nitcbase-debug)
make mode=debug
```

> If you encounter build errors, verify that `libreadline-dev` is installed.

### 3. Run NITCBase

```bash
./nitcbase
```

The engine initialises the disk, loads the buffer pool, and opens the Relation and Attribute Catalogs automatically on startup. You're immediately ready to create tables, insert data, build indexes, and run queries.

---

## 💻 Usage

NITCBase uses a custom SQL-like command syntax. Here's a complete walkthrough:

### Table Management

```sql
-- Create a table (attribute types: NUM or STR)
CREATE TABLE Students(RollNo NUM, Name STR, CGPA NUM);

-- Open and close
OPEN TABLE Students;
CLOSE TABLE Students;

-- Rename a table or an attribute
ALTER TABLE RENAME Students TO UGStudents;
ALTER TABLE RENAME Students COLUMN Name TO FullName;

-- Drop a table
DROP TABLE Students;
```

### Insert Data

```sql
-- Insert a single record
INSERT INTO Students VALUES(1, Alice, 9.5);

-- Bulk insert from a CSV file (place the file in Files/Input_Files/)
INSERT INTO Students VALUES FROM s11students.csv;
```

### Query with Conditions

```sql
-- Select matching records into a new relation (select + project)
SELECT RollNo, Name FROM Students INTO TopStudents WHERE CGPA >= 9.0;

-- Select all attributes (select)
SELECT * FROM Students INTO Output WHERE RollNo = 5;

-- Project specific columns (project)
SELECT RollNo, Name FROM Students INTO Output;

-- Copy the table (project)
SELECT * FROM Students INTO Output;

### Indexing

```sql
-- Build a B+ Tree index on an attribute
CREATE INDEX ON Students.RollNo;

-- Drop the index
DROP INDEX ON Students.RollNo;
```

### Equi-Join

```sql
-- Join two relations on a common attribute into a new relation
SELECT * FROM Students JOIN Courses INTO Output WHERE Students.RollNo = Courses.RollNo;
```

### Batch Scripts

```sql
-- Run a batch script (place .txt files in Files/Batch_Execution_Files/)
RUN s11test.txt;
```

### Exit

```sql
EXIT;
```

> ⚠️ **Note:** NITCBase uses its own command syntax — commands resemble SQL but are **not standard SQL**. For example, `SELECT` always requires an `INTO` clause to store results in a new target relation, and string values are written without quotes, and deleting particular records from a relation also not allowed.

---

## 📊 Implementation Stages

All **12 stages** completed end-to-end:

| Stage | Title | What Was Implemented |
|---|---|---|
| 0 | Installation and Preparation | Project setup, disk initialisation, environment configuration |
| 1 | Understanding the Disk Interface | Block-level read/write on the simulated 16MB disk |
| 2 | Record Blocks and Catalogs | RecBuffer structure, Relation & Attribute Catalog setup |
| 3 | The Disk Buffer and Catalog Caches | 32-block buffer pool, RelCacheTable, AttrCacheTable, OpenRelTable |
| 4 | Linear Search on Relations | `linearSearch()` with condition filtering across record blocks |
| 5 | Opening Relations | `Schema::openRel()`, `closeRel()` — open relation table lifecycle management|
| 6 | Buffer Management and Disk Write-back | LRU replacement via timestamps, dirty-bit writeback on eviction/exit |
| 7 | Inserting Records Into Relations | `Algebra::insert()` with slotmap management and catalog updates |
| 8 | Creating and Deleting Relations | `Schema::createRel()`, `deleteRel()`, `renameRel()`, `renameAttr()` |
| 9 | Selection and Projection on Relations | `Algebra::select()`, `Algebra::project()` (full & partial) |
| 10 | B+ Tree Search on Relations | `bPlusSearch()` — index lookup with leaf-linked list traversal |
| 11 | Index Creation and Deletion | `bPlusInsert()`, `bPlusDelete()`, `Schema::createIndex()`, `Schema::dropIndex()` |
| 12 | Join on Relations | `Algebra::join()` — nested loop equi-join across two relations |

---

## 📖 Documentation

- 📚 [NITCBase Official Documentation](https://nitcbase.github.io/)
- 🖥️ [User Interface Commands Reference](https://nitcbase.github.io/docs/User%20Interface%20Commands/)

---

## 🤝 Contributing

1. Fork the repository
2. Create a new branch: `git checkout -b feature/your-feature`
3. Commit your changes: `git commit -m "Add: short description"`
4. Push to the branch: `git push origin feature/your-feature`
5. Open a Pull Request with a clear description

Please maintain the strict layer separation — each module should only call functions from the layer directly below it.

---

<p align="center">
  Built with 💙 at <strong>NIT Calicut</strong> &nbsp;|&nbsp;
</p>
