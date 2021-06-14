# EulerPowersConjecture
Brute-force approach to generate counterexamples

https://en.wikipedia.org/wiki/Euler%27s_sum_of_powers_conjecture

```math
\sum_{i=1}^{l} a_i^p = b^p \hspace{1em} a_i,b\in\mathbb{Z}_{>0}
```

To avoid repetition, we require $` \mathrm{gcd}(a_1, \cdots, a_l) = 1 `$ and $` b>a_1 \geq a_2 \geq \cdots a_l `$.
If such a solution exists for some length $` l `$ and exponent $` p `$, we put the minimum $` b `$ in the table.
[Fermat's last theorem](https://en.wikipedia.org/wiki/Fermat%27s_Last_Theorem) states that for length=2, exponent>2, there is no solution. 

## Current results (min b)
| length\ exponent 	| 2 	| 3   	| 4     	| 5     	| 6     	|
|----------------	|---	|-----	|-------	|-------	|-------	|
| 2              	| 5 	| ∞   	| ∞     	| ∞     	| ∞     	|
| 3              	| 3 	| 6   	| >1024 	| >1024 	| >1024 	|
| 4              	| 5 	| 7   	| 353   	| 144   	|       	|
| 5              	| 4 	| 4   	| 5     	|       	|       	|
| 6              	| 3 	| 3   	|       	|       	|       	|