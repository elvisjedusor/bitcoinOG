# Bitok Manifesto

## The Short Version

I took Bitcoin v0.3.19, changed the mining algorithm so GPUs can't dominate, gave it a new genesis block, and ran it.

That's the manifesto. You can stop reading here.

---

## The Longer Version (for those who enjoy reading)

### What Happened to Bitcoin

Bitcoin worked in 2010. You downloaded the software, clicked "Generate Coins," and your laptop found blocks. The network grew because participation was free and easy.

Then GPUs happened. Then mining pools. Then FPGAs, ASICs. Then halvings made the reward small enough that only industrial operations made economic sense.

The barrier to entry went from "own a computer" to "have access to cheap electricity and specialized hardware." Most people stopped mining. Many never started.

This was not a conspiracy. It was economics. SHA-256 happens to be very fast on parallel hardware. Someone was always going to optimize it.

### What Satoshi Said About It

December 12, 2010:

> We should have a gentleman's agreement to postpone the GPU arms race as long as we can for the good of the network. It's much easier to get new users up to speed if they don't have to worry about GPU drivers and compatibility. It's nice how anyone with just a CPU can compete fairly equally right now.

To Laszlo Hanyecz (the pizza guy):

> GPUs are much less evenly distributed, so the generated coins only go towards rewarding 20% of the people for joining the network instead of 100%.

And:

> It's inevitable that GPU compute clusters will eventually hog all the generated coins, but I don't want to hasten that day.

He knew. He just didn't have a solution at the time.

### Memory-Hard Algorithms

In 2010, nobody had figured out memory-hard proof-of-work yet. Colin Percival published scrypt in 2009, but it wasn't designed for PoW and had some issues at high parameters.

By 2019, Alexander Peslyak (Solar Designer) had refined the concept into Yespower - a memory-hard algorithm specifically designed to resist GPU and ASIC optimization while remaining efficient on CPUs.

The idea is simple: if every hash requires ~128KB of random memory access, GPUs lose their parallelism advantage. A GPU has thousands of cores but limited memory bandwidth per core. A CPU has fewer cores but can feed each one efficiently.

### What Bitok Changes

Three things:

1. **Yespower instead of SHA-256.** GPU miners don't get 1000x advantage anymore. More like 2x, if that. Not worth the driver headaches.

2. **New genesis block.** Separate network. No confusion with BTC. No replay attacks. Clean slate.

3. **Modern build system.** The original code needed patches to compile on anything newer than Ubuntu 10.04. Now it builds on Ubuntu 24.04, macOS, Windows.

Everything else is exactly as Satoshi left it. Same 21 million cap. Same 10 minute blocks. Same halving schedule. Same transaction format. Same script system. Same wallet behavior.

### What Bitok Does Not Change

- No new opcodes
- No SegWit
- No block size increase
- No layer 2
- No BIPs
- No governance
- No foundation
- No roadmap
- No improvement proposals

The protocol is frozen at December 2010. If you think Bitcoin needed all those changes to succeed, this probably isn't for you.

### Why

I wanted to mine Bitcoin with my laptop. Not buy ASICs. Not join pools. Not pay for cloud hashrate.

I couldn't, because of physics and economics.

So I made this instead.

### The Philosophy Part (if you want one)

Bitcoin's whitepaper is 9 pages. Most of it is math. Satoshi didn't write manifestos. He wrote code that worked and let people figure out what it meant.

The code said: anyone can transact. Anyone can mine. No intermediaries. No permission.

Somewhere along the way, "anyone can mine" became "anyone with industrial-scale infrastructure can mine." That happened gradually and probably inevitably.

Bitok is an experiment in what Bitcoin would look like if the mining algorithm had been different from the start. Nothing more profound than that.

### Who Is Tom Elvis Jedusor

An anagram. A Harry Potter reference. A pseudonym.

Why does it matter?

### Will This Succeed

Define "succeed."

If you mean: will the software compile and run? Yes. I tested it.

If you mean: will people use it? Some might. Most won't. That's fine.

If you mean: will it be worth money? I genuinely don't know. Probably not much. Markets are weird and I don't predict them.

If you mean: will it stay decentralized? That depends on whether enough people run nodes. I can't control that. Neither can you. That's the point.

### No Promises

I'm not going to tell you this will change the world. I'm not going to tell you to buy it. I'm not going to tell you anything you want to hear just because it sounds good.

Here's software. It does what it says. Run it or don't.

---

## Technical Specifications

```
Algorithm:          Yespower 1.0 (N=2048, r=32)
Block time:         10 minutes
Block reward:       50 BITOK (halves every 210,000 blocks)
Max supply:         21,000,000 BITOK
Difficulty adjust:  every 2016 blocks
Coinbase maturity:  100 blocks
P2P port:           18333
RPC port:           8332
```

## Security Fixes Included

All security fixes from Bitcoin v0.3.19 are present:

- Value overflow protection (the 184 billion coin bug)
- Blockchain checkpoints
- DoS limits
- IsStandard() transaction filtering

Bitcoin forked once in August 2010 to fix the overflow bug. Bitok launches with all fixes in place. No forks needed.

## Source

https://github.com/elvisjedusor/bitok

---

Run the code. That is the manifesto.
