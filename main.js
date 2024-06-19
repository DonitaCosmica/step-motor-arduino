let isSending = false

document.addEventListener("DOMContentLoaded", () => {
  const resources = [
    document.getElementById('favicon'),
    document.getElementById('stepMotorGif')
  ]

  const checkResourceLoad = () => {
    const allLoaded = resources.every(res => {
      if (res.tagName === 'IMG')
        return res.complete
      else if (res.tagName === 'LINK' && res.rel === 'icon') {
        const img = new Image()
        img.src = res.href
        return img.complete
      }
      return false
    })

    if (allLoaded && !isSending)
      getPercentage()
  }

  const getPercentage = async () => {
    const res = await fetch('/per')
    const data = await res.json()

    const { percentage } = data
    const percentageText = document.getElementById('percentage')
    percentageText.textContent = `${percentage}%`
  }

  checkResourceLoad()
  setInterval(() => {
    if(!isSending)
      getPercentage()
  }, 4000)
})

const sendTurns = async () => {
  const turns = document.getElementById('turns')
  const turnValue = turns.value

  if (isNaN(turnValue)) {
    alert('Por favor ingrese un numero valido')
    return
  }
      
  try {
    isSending = true
    const res = await fetch(`/tur?turns=${turnValue}`)
    if (!res.ok) throw new Error('Network response was not ok')
    console.log('Data has been sent successfully')
  } catch (error) {
    console.error('Data has not been sent successfully')
  } finally {
    isSending = false
  }
}

const turnForm = document.getElementById('turnForm')
turnForm.addEventListener('submit', (event) => event.preventDefault())