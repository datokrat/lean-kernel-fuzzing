prelude
import Init.Core

namespace Classical2

noncomputable def indefiniteDescription {α : Sort u} (p : α → Prop) (h : Exists p) : {x // p x} :=
  Classical.choice <| let ⟨x, px⟩ := h; ⟨⟨x, px⟩⟩

/--
Given that there exists an element satisfying `p`, returns one such element.

This is a straightforward consequence of, and equivalent to, `Classical.choice`.

See also `choose_spec`, which asserts that the returned value has property `p`.
-/
noncomputable def choose {α : Sort u} {p : α → Prop} (h : Exists p) : α :=
  (indefiniteDescription p h).val

theorem choose_spec {α : Sort u} {p : α → Prop} (h : Exists p) : p (choose h) :=
  (indefiniteDescription p h).property

/-- **Diaconescu's theorem**: excluded middle from choice, Function extensionality and propositional extensionality. -/
theorem em (p : Prop) : p ∨ ¬p :=
  let U (x : Prop) : Prop := x = True ∨ p
  let V (x : Prop) : Prop := x = False ∨ p
  have exU : Exists U := ⟨True, Or.inl rfl⟩
  have exV : Exists V := ⟨False, Or.inl rfl⟩
  let u : Prop := choose exU
  let v : Prop := choose exV
  have u_def : U u := choose_spec exU
  have v_def : V v := choose_spec exV
  have not_uv_or_p : u ≠ v ∨ p :=
    match u_def, v_def with
    | Or.inr h, _ => Or.inr h
    | _, Or.inr h => Or.inr h
    | Or.inl hut, Or.inl hvf =>
      have hne : u ≠ v := by
        rw [hut, hvf]
        apply true_ne_false
      Or.inl hne
  have p_implies_uv : p → u = v :=
    fun hp =>
    have hpred : U = V :=
      funext fun x =>
        have hl : (x = True ∨ p) → (x = False ∨ p) :=
          fun _ => Or.inr hp
        have hr : (x = False ∨ p) → (x = True ∨ p) :=
          fun _ => Or.inr hp
        show (x = True ∨ p) = (x = False ∨ p) from
          propext (Iff.intro hl hr)
    have h₀ : ∀ exU exV, @choose _ U exU = @choose _ V exV := by
      rw [hpred]; intros; rfl
    show u = v from h₀ _ _
  match not_uv_or_p with
  | Or.inl hne => Or.inr (mt p_implies_uv hne)
  | Or.inr h   => Or.inl h
